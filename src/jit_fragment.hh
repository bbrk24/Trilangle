#pragma once

#include "compat.hh"

#if x86_64_JIT_ALLOWED
#include "instruction.hh"

class jit_fragment {
public:
    jit_fragment(const std::vector<instruction>& fragment);

    inline void execute(NONNULL_PTR(std::vector<int24_t>) stack) const { m_func_ptr(stack, methods); }
    inline const instruction& last_instruction() const noexcept { return m_end; }
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

    static const vector_method methods[4];

    void (*m_func_ptr)(std::vector<int24_t>*, const vector_method*);
};

const jit_fragment::vector_method jit_fragment::methods[] = {
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

#endif
