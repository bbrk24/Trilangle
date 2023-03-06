#pragma once

#include "input.hh"
#include "program_walker.hh"

class thread : public program_walker {
public:
    static unsigned long thread_count;

    enum class status : char {
        active,      // Currently executing code
        idle,        // Inactive for a single tick due to e.g. a "skip" operation
        splitting,   // Arrived at a thread split and waiting to be split in two
        waiting,     // Waiting for a second thread to merge with
        terminated,  // The thread is no longer executing
    };

    CONSTEXPR_ALLOC thread(const program& p, flags f) noexcept :
        program_walker(p),
        m_stack(),
        m_ip{ { SIZE_C(0), SIZE_C(0) }, direction::southwest },
        m_status(status::active),
        m_flags(f),
        m_number(thread_count++) {}

    CONSTEXPR_ALLOC thread(const thread& other, direction d) noexcept :
        program_walker(*other.m_program),
        m_stack(other.m_stack),
        m_ip{ other.m_ip.coords, d },
        m_status(status::active),
        m_flags(other.m_flags),
        m_number(thread_count++) {
        advance();
    }

    constexpr status get_status() const noexcept { return m_status; }
    constexpr const instruction_pointer& get_ip() const noexcept { return m_ip; }
    constexpr void advance() noexcept { program_walker::advance(m_ip, m_program->side_length()); }

    void tick();
private:
    std::vector<int24_t> m_stack;
    instruction_pointer m_ip;
    status m_status;
    flags m_flags;
    unsigned long m_number;
};
