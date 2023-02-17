#pragma once

#include "int24.hh"
#include "string_processing.hh"
#include "opcode.hh"
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>

constexpr size_t triangular(size_t n) noexcept {
    return n * (n + 1) / 2;
}

class program {
public:
    program() = default;

    CONSTEXPR_ALG program(const std::string& source) : m_code(parse_utf8(source, true)), m_side_length(0) {
        // remove all whitespace
        auto iter = std::remove_if(
            m_code.begin(),
            m_code.end(),
            [](int24_t c) {
                return c == (int24_t)' ' || c == (int24_t)'\n';
            }
        );
        m_code.erase(iter, m_code.end());

        // Determine the next triangular number
        size_t capacity;
        do {
            ++m_side_length;
            capacity = triangular(m_side_length);
        } while (capacity < m_code.size());

        // Fill the remaining space with NOPs
        m_code.resize(capacity, static_cast<int24_t>(opcode::NOP));
    }

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
