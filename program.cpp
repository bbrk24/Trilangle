#include "program.hh"
#include "string_processing.hh"
#include <algorithm>
#include <cassert>

using std::string;

static inline size_t triangular(size_t n) {
    return n * (n + 1) / 2;
}

program::program(const string& source) : m_code(parse_utf8(source)), m_side_length(0) {
    // remove all whitespace

    auto iter = std::stable_partition(
        m_code.begin(),
        m_code.end(),
        [](int24_t c) {
            return c != ' ' && c != '\n';
        }
    );
    m_code.erase(iter, m_code.end());

    size_t capacity;
    do {
        ++m_side_length;
        capacity = triangular(m_side_length);
    } while (capacity < m_code.size());

    m_code.reserve(capacity);

    while (m_code.size() < capacity) {
        m_code.emplace_back('.');
    }
}

int24_t program::at(size_t row, size_t column) const {
    assert(row <= m_side_length && column <= row);

    size_t idx = triangular(row) + column;
    return m_code[idx];
}
