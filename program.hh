#pragma once

#include "int24.hh"
#include <vector>
#include <string>
#include <cassert>

constexpr size_t triangular(size_t n) noexcept {
    return n * (n + 1) / 2;
}

class program {
public:
    program() = default;
    program(const std::string& source);

    CONSTEXPR_ALLOC int24_t at(size_t row, size_t column) const noexcept {
        assert(row < m_side_length&& column <= row);

        size_t idx = triangular(row) + column;
        return m_code[idx];
    }

    constexpr size_t side_length() const noexcept { return m_side_length; }
private:
    std::vector<int24_t> m_code;
    size_t m_side_length;
};
