#pragma once

#include <algorithm>
#include "opcode.hh"
#include "string_processing.hh"

constexpr size_t triangular(size_t n) noexcept {
    return n * (n + 1) / 2;
}

struct program {
    program() = default;

    CONSTEXPR_ALG program(const std::string& source) :
        m_code(parse_utf8(
            source,
#ifdef __EMSCRIPTEN__
            false
#else
            true
#endif
        )) {
        // // remove all whitespace
        // auto iter = std::remove_if(m_code.begin(), m_code.end(), [](int24_t c) {
        //     return c == (int24_t)' ' || c == (int24_t)'\n';
        // });
        // m_code.erase(iter, m_code.end());
    }

    CONSTEXPR_VECTOR int24_t at(size_t row, size_t column) const noexcept {
        assert(column <= row);

        size_t idx = triangular(row) + column;

        if (idx >= m_code.size()) {
            return static_cast<int24_t>(opcode::NOP);
        }

        return m_code[idx];
    }

    constexpr size_t side_length() const noexcept {
        size_t length = 0;
        for (size_t i = 0; length < m_code.size(); ++i) {
            length += i;
        }
        return length;
    }

    std::vector<int24_t> m_code;
};

std::ostream& operator<<(std::ostream& lhs, const program& rhs);
