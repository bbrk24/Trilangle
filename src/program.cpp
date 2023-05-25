#include "program.hh"
#include "output.hh"

std::ostream& operator<<(std::ostream& lhs, const program& rhs) {
    for (size_t row = 0; row < rhs.side_length(); ++row) {
        for (size_t i = 0; i < rhs.side_length() - row - 1; ++i) {
            lhs << ' ';
        }

        for (size_t col = 0; col <= row; ++col) {
            printunichar(rhs.at(row, col), lhs);
            lhs << (col == row ? '\n' : ' ');
        }
    }

    return lhs;
}
