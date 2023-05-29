#pragma once

#include "../compat.hh"

#if x86_64_JIT_ALLOWED
#include "../instruction.hh"
namespace jit {

class fragment {
public:
    fragment(const std::vector<instruction>& fragment);

    inline void execute(NONNULL_PTR(std::vector<int24_t>) stack) const { m_func_ptr(stack, methods); }
    inline const instruction& last_instruction() const noexcept { return m_end; }
    constexpr bool is_valid() const noexcept { return m_func_ptr != nullptr; }
private:
    instruction m_end;

    // Calling conventions, amirite?
    // Ensure that the vector is in RCX and the integer is in RDX regardless of ABI
#ifdef _WIN64
#define VECTOR_METHOD_ARGS std::vector<int24_t>*vec, MAYBE_UNUSED intptr_t i
#else
#define VECTOR_METHOD_ARGS intptr_t, intptr_t, MAYBE_UNUSED intptr_t i, std::vector<int24_t>*vec
#endif

    typedef intptr_t (*vector_method)(VECTOR_METHOD_ARGS);

    typedef void (*exec_fptr)(std::vector<int24_t>*, const vector_method*);

    static exec_fptr make_executable(const void* mem, size_t length);
    static const vector_method methods[4];

    exec_fptr m_func_ptr;
};

}  // namespace jit
#endif
