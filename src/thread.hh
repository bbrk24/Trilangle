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
        waiting,     // Waiting for a second thread to merge with
        splitting,   // Arrived at a thread split and waiting to be split in two
        terminated,  // The thread is no longer executing
    };

    // Currently only called with T = std::vector<int24_t> and T = const std::vector<int24_t>&.
    // Generic to allow both move- and copy-construction.
    template<typename T>
    CONSTEXPR_VECTOR thread(const thread& other, direction d, T&& stack) noexcept :
        program_walker(other.m_program),
        m_stack(std::forward<T>(stack)),
        m_ip{ other.m_ip.coords, d },
        m_status(status::active),
        m_flags(other.m_flags),
        m_number(++thread_count) {
        advance();
    }

    CONSTEXPR_VECTOR thread(const thread& other, direction d) noexcept : thread(other, d, other.m_stack) {}

    void tick();
protected:
    CONSTEXPR_VECTOR thread(NONNULL_PTR(const program) p, flags f) noexcept :
        program_walker(p),
        m_stack(),
        m_ip{ { SIZE_C(0), SIZE_C(0) }, direction::southwest },
        m_status(status::active),
        m_flags(f),
        m_number(thread_count++) {}

    constexpr void advance() noexcept { program_walker::advance(m_ip, m_program->side_length()); }

    std::vector<int24_t> m_stack;
    NO_UNIQUE_ADDRESS instruction_pointer m_ip;
    status m_status;
    flags m_flags;
private:
    unsigned long m_number;
};
