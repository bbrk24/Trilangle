#pragma once

#include "input.hh"
#include "program_walker.hh"

class thread : public program_walker {
    friend class interpreter;
public:
    static unsigned long thread_count;

    enum class status : char {
        active,      // Currently executing code
        idle,        // Inactive for a single tick due to e.g. a "skip" operation
        terminated,  // The thread is no longer executing
    };

    // Currently only called with T = std::vector<int24_t> and T = const std::vector<int24_t>&.
    // Generic to allow both move- and copy-construction.
    template<typename T>
    thread(const thread& other, direction d, T& stack) noexcept :
        program_walker(other.m_program),
        m_stack(stack),
        m_ip{ other.m_ip.coords, d },
        m_status(status::active),
        m_flags(other.m_flags),
        m_number(++thread_count) {
        advance();
    }

    inline thread(const thread& other, direction d) noexcept : thread(other, d, other.m_stack) {}

    void tick();
protected:
    inline thread(NONNULL_PTR(program) p, flags f) noexcept :
        program_walker(p),
        m_stack(p->m_code),
        m_ip{ { SIZE_C(0), SIZE_C(0) }, direction::southwest },
        m_status(status::active),
        m_flags(f),
        m_number(thread_count++) {}

    constexpr void advance() noexcept { program_walker::advance(m_ip, m_program->side_length()); }

    std::vector<int24_t>& m_stack;
    NO_UNIQUE_ADDRESS instruction_pointer m_ip;
    status m_status;
    flags m_flags;

    static_assert(
        sizeof(instruction_pointer) % sizeof(size_t) + sizeof(status) + sizeof(flags) <= sizeof(size_t),
        "flags, status, or instruction_pointer got too big, adding an entire extra word of padding"
    );
private:
    unsigned long m_number;
};

extern "C" void send_debug_info(
    unsigned long thread_number,
    const int24_t* stack,
    size_t stack_depth,
    size_t y,
    size_t x,
    int32_t instruction
);
