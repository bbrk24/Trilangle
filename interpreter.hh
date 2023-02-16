#pragma once

#include "program.hh"
#include "input.hh"
#include <utility>

enum class direction : char {
    southwest, west, northwest, northeast, east, southeast
};

class interpreter {
public:
    interpreter(const program& p, flags f) noexcept;

    void run();
private:
    void advance() noexcept;

    std::vector<int24_t> m_stack;
    std::pair<size_t, size_t> m_coords;
    const program& m_program;
    flags m_flags;
    direction m_direction;
};
