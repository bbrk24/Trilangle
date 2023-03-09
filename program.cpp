#include "program.hh"
#include <sstream>
#include "output.hh"

std::string program::with_spaces() const {
    std::ostringstream oss;

    for (size_t row = 0; row < m_side_length; ++row) {
        for (size_t i = 0; i < m_side_length - row - 1; ++i) {
            oss << ' ';
        }

        for (size_t col = 0; col <= row; ++col) {
            printunichar(at(row, col), oss);
            oss << (col == row ? '\n' : ' ');
        }
    }

    return oss.str();
}
