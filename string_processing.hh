#pragma once

#include "int24.hh"
#include <vector>
#include <string>
#include <cstdio>

constexpr int24_t INVALID_CHAR{ 0xfffd };

// A type suitable for storing the index of a C string.
// Not necessarily the same as std::string::size_type, and not necessarily capable of holding negative values.
typedef size_t string_index;

// Given a function that yields one "character" at a time, parse the incoming bytestream as UTF-8, and return a single
// Unicode character.
template<typename FuncType>
int24_t parse_unichar(FuncType getbyte) noexcept(noexcept(getbyte())) {
    unsigned char buf[4];

    // The maximum number of used bytes in the buffer, determined by the first byte read.
    string_index buf_max = 4;
    for (string_index i = 0; i < buf_max; ++i) {
        int c = getbyte();

        // Handle EOF
        if (c == EOF) {
            if (i == 0) {
                // The character started as EOF
                return INT24_C(-1);
            } else {
                // The EOF came mid-character
                return INVALID_CHAR;
            }
        }


        buf[i] = static_cast<unsigned char>(c);
        if (i != 0 && (buf[i] & 0xc0) != 0x80) {
            return INVALID_CHAR;
        }

        if (i == 0) {
            if (buf[0] < 0x80) {
                return buf[0];
            } else if ((buf[0] & 0xe0) == 0xc0) {
                buf_max = 2;
            } else if ((buf[0] & 0xf0) == 0xe0) {
                buf_max = 3;
            } else if ((buf[0] & 0xf8) != 0xf0) {
                return INVALID_CHAR;
            }
        }
    }

    // Perform bitwise math to extract the actual value out of the UTF-8.
    switch (buf_max) {
        case 2:
            return static_cast<int24_t>(((buf[0] & 0x1f) << 6) | (buf[1] & 0x3f));
        case 3:
            return static_cast<int24_t>(((buf[0] & 0x0f) << 12) | ((buf[1] & 0x3f) << 6) | (buf[2] & 0x3f));
        case 4:
            return static_cast<int24_t>(
                ((buf[0] & 0x07) << 18) | ((buf[1] & 0x3f) << 12) | ((buf[2] & 0x3f) << 6) | (buf[3] & 0x3f)
            );
        default:
            unreachable();
    }
}

CONSTEXPR_ALLOC std::vector<int24_t> parse_utf8(const std::string& s, bool skip_shebang) {
    auto iter = s.begin();

    std::vector<int24_t> vec;
    vec.reserve(s.size() / 4);

    if (skip_shebang && s.size() > 2 && s[0] == '#' && s[1] == '!') {
        while (*iter++ != '\n');
    }

    do {
        vec.push_back(
            parse_unichar([&]() NOEXCEPT_T {
                if (iter == s.end()) {
                    return EOF;
                } else {
                    return static_cast<int>(*iter++);
                }
            })
        );
    } while (iter != s.end());

    return vec;
}