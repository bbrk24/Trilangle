#pragma once

#include "program_walker.hh"
#include "input.hh"

class interpreter : public program_walker {
public:
    CONSTEXPR_ALLOC interpreter(const program& p, flags f) noexcept : program_walker(p),
        m_stack(),
        m_ip{ { SIZE_C(0), SIZE_C(0) }, direction::southwest },
        m_flags(f) { }

    void run();

    static void stop_all() noexcept;
private:
    constexpr void advance() noexcept {
        program_walker::advance(m_ip, m_program.side_length());
    }

    std::vector<int24_t> m_stack;
    instruction_pointer m_ip;
    const flags m_flags;
};
