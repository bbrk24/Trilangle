#pragma once

#include "int24.hh"
#include <vector>
#include <string>

class program {
public:
    program() = default;
    program(const std::string& source);

    int24_t at(size_t row, size_t column) const;

    constexpr size_t side_length() const noexcept { return m_side_length; }
private:
    std::vector<int24_t> m_code;
    size_t m_side_length;
};
