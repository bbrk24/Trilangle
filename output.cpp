#define _CRT_SECURE_NO_WARNINGS 1

#include "output.hh"
#include <cinttypes>
#include <cstdio>

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
    } else if (c.value <= 0x1fffff) {
        char buffer[] = {
            static_cast<char>(0xf0 | (c.value >> 18)),
            static_cast<char>(0x80 | ((c.value >> 12) & 0x3f)),
            static_cast<char>(0x80 | ((c.value >> 6) & 0x3f)),
            static_cast<char>(0x80 | (c.value & 0x3f)),
            0,
        };
        os << buffer;
    } else UNLIKELY {
        std::cerr << std::flush;
        fprintf(stderr, "Error: 0x%" PRIx32 " is too large to express in UTF-8.", c.value);
    }
}
