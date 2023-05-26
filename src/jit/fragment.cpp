#include "fragment.hh"

#if x86_64_JIT_ALLOWED

using jit::fragment;

#ifdef _WIN64
#include <memoryapi.h>

fragment::exec_fptr fragment::make_executable(const void* mem, size_t length) {
    DWORD protect = PAGE_READWRITE;
    LPVOID loc = VirtualAlloc(nullptr, length, MEM_COMMIT | MEM_RESERVE, protect);
    if (loc == nullptr) {
        return nullptr;
    }

    memcpy(loc, mem, length);

    BOOL success = VirtualProtect(loc, length, PAGE_EXECUTE, &protect);
    if (!success) {
        VirtualFree(loc, 0, MEM_RELEASE);
        return nullptr;
    }

    return reinterpret_cast<exec_fptr>(loc);
}
#else
#include <sys/mman.h>

fragment::exec_fptr fragment::make_executable(const void* mem, size_t length) {
    void* loc = mmap(nullptr, length, PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (loc == MAP_FAILED) {
        return nullptr;
    }

    memcpy(loc, mem, length);

    int failed = mprotect(loc, length, PROT_EXEC);
    if (failed) {
        munmap(loc, length);
        return nullptr;
    }

    return reinterpret_cast<exec_fptr>(loc);
}
#endif

const fragment::vector_method fragment::methods[] = {
    // push
    [](VECTOR_METHOD_ARGS) -> intptr_t {
        vec->emplace_back(i);
        return 0;
    },
    // pop
    [](VECTOR_METHOD_ARGS) NOEXCEPT_T {
        int24_t val = vec->back();
        vec->pop_back();
        return static_cast<intptr_t>(val);
    },
    // index
    [](VECTOR_METHOD_ARGS) { return static_cast<intptr_t>(vec->at(vec->size() - i - 1)); },
    // peek
    [](VECTOR_METHOD_ARGS) { return static_cast<intptr_t>(vec->back()); }
};

fragment::fragment(const std::vector<instruction>& fragment) : m_end(fragment.back()), m_func_ptr(nullptr) {
    using operation = instruction::operation;

    // Reserving this many instructions is _probably_ not the correct number but it's better than nothing, I think.
    std::vector<uint8_t> instructions;
    instructions.reserve(fragment.size());

    // MSVC ABI: args are in RCX,RDX; nonvolatile regs are RBX,RBP,RDI,RSI,RSP,R12,R13,R14,R15
    // SystemV ABI: args are in RDI,RSI; nonvolatile regs are RBX,RSP,RBP,R12,R13,R14,R15
    // SystemV nonvolatile registers are a subset of MSVC nonvolatile registers.

    // Preamble
    instructions.insert(
        instructions.end(),
        { // push rbp
          0x55,
          // push rbx
          0x53,
#ifdef _WIN64
          // mov rbp,rdx
          0x48,
          0x89,
          0xd5,
          // mov rbx,rcx
          0x48,
          0x89,
          0xcb
#else
          // mov rbp,rsi
          0x48,
          0x89,
          0xf5,
          // mov rbx,rdi
          0x48,
          0x89,
          0xfb
#endif
        }
    );

    // Okay. Now the vector is in RBX and the method table is in RBP

    auto alloc_shadow_space = [&]() {
#ifdef _WIN64
        // Cancel out paired shadow space alloc/dealloc
        if (
            // add rsp,40
            instructions[instructions.size() - 4] == 0x48 &&
            instructions[instructions.size() - 3] == 0x83 &&
            instructions[instructions.size() - 4] == 0xc4 &&
            instructions[instructions.size() - 1] == 0x28
        ) {
            instructions.erase(instructions.end() - 4, instructions.end());
        } else {
            instructions.insert(
                instructions.end(),
                { // sub rsp,40
                  0x48,
                  0x83,
                  0xec,
                  0x28 }
            );
        }
#endif
    };

    // Pop-operate-push type instructions, like INC
    // Argument is in RAX and return value goes in RDX
    auto emit_one_arg_op = [&](std::initializer_list<uint8_t> i) {
        alloc_shadow_space();
        instructions.insert(
            instructions.end(),
            { // mov rcx,rbx
              0x48,
              0x89,
              0xd9,
              // call QWORD PTR [rbp+8]
              0xff,
              0x55,
              0x08 }
        );
        instructions.insert(instructions.end(), i);
        instructions.insert(
            instructions.end(),
            { // mov rcx,rbx
              0x48,
              0x89,
              0xd9,
              // call QWORD PTR [rbp]
              0xff,
              0x55,
              0x00
#ifdef _WIN64
              ,
              // add rsp,40
              0x48,
              0x83,
              0xc4,
              0x28
#endif
            }
        );
    };

    // Pop-pop-operate-push type instructions, like ADD
    // Arguments are in R12 and RAX, return value goes in RDX
    auto emit_two_arg_op = [&](std::initializer_list<uint8_t> i) {
        alloc_shadow_space();
        instructions.insert(
            instructions.end(),
            { // mov rcx,rbx
              0x48,
              0x89,
              0xd9,
              // call QWORD PTR [rbp+8]
              0xff,
              0x55,
              0x08,
#ifdef _WIN64
              // add rsp,40
              0x48,
              0x83,
              0xc4,
              0x28,
#endif
              // push r12
              0x41,
              0x54,
              // mov r12,rax
              0x49,
              0x89,
              0xc4,
              // mov rcx,rbx
              0x49,
              0x89,
              0xd9,
#ifdef _WIN64
              // sub rsp,32
              0x48,
              0x83,
              0xec,
              0x20,
#endif
              // call QWORD PTR [rbp+8]
              0xff,
              0x55,
              0x08 }
        );
        instructions.insert(instructions.end(), i);
        instructions.insert(
            instructions.end(),
            {
#ifdef _WIN64
                // add rsp,32
                0x48,
                0x83,
                0xc4,
                0x20,
#endif
                // pop r12
                0x41,
                0x5c,
#ifdef _WIN64
                // sub rsp,40
                0x48,
                0x83,
                0xec,
                0x28,
#endif
                // call QWORD PTR [rbp]
                0xff,
                0x55,
                0x00,
#ifdef _WIN64
                // add rsp,40
                0x48,
                0x83,
                0xc4,
                0x28
#endif
            }
        );
    };

    for (size_t i = 0; i < fragment.size() - 1; ++i) {
        const instruction& instr = fragment[i];
        switch (instr.m_op) {
            // Don't emit actual NOPs
            case operation::NOP:
                continue;
            case operation::DEC:
                // lea rdx,[rax-1]
                emit_one_arg_op({ 0x48, 0x8d, 0x50, 0xff });
                break;
            case operation::INC:
                // lea rdx,[rax+1]
                emit_one_arg_op({ 0x48, 0x8f, 0x50, 0x01 });
                break;
            case operation::EXP:
                emit_one_arg_op({ // mov rdx,1
                                  0x48,
                                  0xc7,
                                  0xc2,
                                  0x01,
                                  0x00,
                                  0x00,
                                  0x00,
                                  // mov cl,al
                                  0x88,
                                  0xc1,
                                  // shl rdx,cl
                                  0x48,
                                  0xd3,
                                  0xe2 });
                break;
            case operation::NOT:
                emit_one_arg_op({ // not rax
                                  0x48,
                                  0xf7,
                                  0xd0,
                                  // mov rdx,rax
                                  0x48,
                                  0x89,
                                  0xc2 });
                break;
            case operation::ADD:
                // lea rdx,[rax+r12]
                emit_two_arg_op({ 0x4a, 0x8d, 0x14, 0x20 });
                break;
            case operation::AND:
                emit_two_arg_op({ // and rax,r12
                                  0x4c,
                                  0x21,
                                  0xe0,
                                  // mov rdx,rax
                                  0x48,
                                  0x89,
                                  0xc2 });
                break;
            case operation::DIV:
                emit_two_arg_op({ // cdq
                                  0x99,
                                  // idiv r12d
                                  0x41,
                                  0xf7,
                                  0xfc,
                                  // movsx rdx,eax
                                  0x48,
                                  0x63,
                                  0xd0 });
                break;
            case operation::MOD:
                emit_two_arg_op({ // cdq
                                  0x99,
                                  // idiv r12d
                                  0x41,
                                  0xf7,
                                  0xfc,
                                  // movsx rdx,edx
                                  0x48,
                                  0x63,
                                  0xd2 });
                break;
            case operation::IOR:
                emit_two_arg_op({ // or rax,r12
                                  0x4c,
                                  0x09,
                                  0xe0,
                                  // mov rdx,rax
                                  0x48,
                                  0x89,
                                  0xc2 });
                break;
            case operation::XOR:
                emit_two_arg_op({ // xor rax,r12
                                  0x4c,
                                  0x31,
                                  0xe0,
                                  // mov rdx,rax
                                  0x48,
                                  0x89,
                                  0xc2 });
                break;
            case operation::MUL:
                emit_two_arg_op({ // imul eax,r12d
                                  0x41,
                                  0x0f,
                                  0xaf,
                                  0xc4,
                                  // movsx rdx,eax
                                  0x48,
                                  0x63,
                                  0xd0 });
                break;
            case operation::SUB:
                emit_two_arg_op({ // sub rax,r12
                                  0x4c,
                                  0x29,
                                  0xe0,
                                  // mov rdx,rax
                                  0x48,
                                  0x89,
                                  0xc2 });
                break;
            case operation::IDX:
                emit_one_arg_op({ // mov rdx,rax
                                  0x48,
                                  0x89,
                                  0xc2,
                                  // mov rcx,rbx
                                  0x48,
                                  0x89,
                                  0xd9,
                                  // call QWORD PTR [rbp+16]
                                  0xff,
                                  0x55,
                                  0x10,
                                  // mov rdx,rax
                                  0x48,
                                  0x89,
                                  0xc2 });
                break;
            // TODO: DUP, DP2, SWP, POP, PSC, PSI
            default:
                // some instructions can't be compiled.
                return;
        }
    }

    // Postamble
    instructions.insert(
        instructions.end(),
        { // pop rbx
          0x5b,
          // pop rbp
          0x5d,
#ifdef _WIN64
          // ret 0
          0xc2,
          0x00,
          0x00
#else
          // ret
          0xc3
#endif
        }
    );

    m_func_ptr = make_executable(instructions.data(), instructions.size());
}
#endif
