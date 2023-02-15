#pragma once

#include "int24.hh"
#include <vector>
#include <string>
#include <cstdio>

constexpr int24_t INVALID_CHAR{ 0xfffd };

// Given a function that yields one "character" at a time, parse the incoming bytestream as UTF-8, and return a single
// Unicode character.
template<typename FuncType>
int24_t parse_unichar(FuncType getbyte) {
    unsigned char buf[4];

    // The maximum number of used bytes in the buffer, determined by the first byte read.
    uintptr_t buf_max = 4;
    for (uintptr_t i = 0; i < buf_max; ++i) {
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
            return INVALID_CHAR;
    }
}

std::vector<int24_t> parse_utf8(const std::string& s) noexcept;
