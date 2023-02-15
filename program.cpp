#include "program.hh"
#include "string_processing.hh"
#include "opcode.hh"
#include <algorithm>
#include <cassert>

constexpr size_t triangular(size_t n) {
    return n * (n + 1) / 2;
}

program::program(const std::string& source) : m_code(parse_utf8(source)), m_side_length(0) {
    // remove all whitespace
    auto iter = std::stable_partition(
        m_code.begin(),
        m_code.end(),
        [](int24_t c) {
            return c != (int24_t)' ' && c != (int24_t)'\n';
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
    m_code.reserve(capacity);
    while (m_code.size() < capacity) {
        m_code.emplace_back(opcode::NOP);
    }
}

int24_t program::at(size_t row, size_t column) const {
    assert(row < m_side_length && column <= row);

    size_t idx = triangular(row) + column;
    return m_code[idx];
}
