#pragma once

#include "program.hh"
#include "input.hh"

enum class direction : char {
    southwest, west, northwest, northeast, east, southeast
};

class interpreter {
public:
    CONSTEXPR_ALLOC interpreter(const program& p, flags f) noexcept :
        m_stack(),
        m_coords{ SIZE_C(0), SIZE_C(0) },
        m_program(p),
        m_flags(f),
        m_direction(direction::southwest) { }

    void run();
private:
    void advance() noexcept;

    std::vector<int24_t> m_stack;
    std::pair<size_t, size_t> m_coords;
    const program& m_program;
    flags m_flags;
    direction m_direction;
};
