#include "program.hh"
#include "string_processing.hh"
#include "opcode.hh"
#include <algorithm>

program::program(const std::string& source) : m_code(parse_utf8(source, true)), m_side_length(0) {
    // remove all whitespace
    auto iter = std::stable_partition(
        m_code.begin(),
        m_code.end(),
        [](int24_t c) NOEXCEPT_T {
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
    m_code.resize(capacity, static_cast<int24_t>(opcode::NOP));
}
