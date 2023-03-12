#include "output.hh"

void printunichar(int24_t c, std::ostream& os) {
    // Convert to UTF-8
    if (c.value <= 0x7f) {
        os << static_cast<char>(c.value);
    } else if (c.value <= 0x07ff) {
        char buffer[] = {
            static_cast<char>(0xc0 | (c.value >> 6)),
            static_cast<char>(0x80 | (c.value & 0x3f)),
            0,
        };
        os << buffer;
    } else if (c.value <= 0xffff) {
        char buffer[] = {
            static_cast<char>(0xe0 | (c.value >> 12)),
            static_cast<char>(0x80 | ((c.value >> 6) & 0x3f)),
            static_cast<char>(0x80 | (c.value & 0x3f)),
            0,
        };
        os << buffer;
    } else {
        char buffer[] = {
            static_cast<char>(0xf0 | (c.value >> 18)),
            static_cast<char>(0x80 | ((c.value >> 12) & 0x3f)),
            static_cast<char>(0x80 | ((c.value >> 6) & 0x3f)),
            static_cast<char>(0x80 | (c.value & 0x3f)),
            0,
        };
        os << buffer;
    }

    os.clear();
}
