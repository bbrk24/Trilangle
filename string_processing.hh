#pragma once

#include "int24.hh"
#include <type_traits>
#include <vector>
#include <string>

constexpr int24_t INVALID_CHAR{ 0xfffd };

// Given a function that yields one "character" at a time, parse the incoming bytestream as UTF-8.
template<typename FuncType>
int24_t parse_unichar(FuncType getbyte) {
    unsigned char buf[4];

    size_t buf_max = 4;
    for (size_t i = 0; i < buf_max; ++i) {
        int c = getbyte();
        buf[i] = (unsigned char)c;

        if (c == EOF) {
            if (i == 0) {
                return INT24_C(-1);
            } else {
                break;
            }
        }


        if (i != 0 && (buf[i] & 0xc0) != 0x80) {
            return INVALID_CHAR;
        }

        if (buf[i] < 0x80) {
            break;
        }
        if (i == 0) {
            if ((buf[0] & 0xe0) == 0xc0) {
                buf_max = 2;
            } else if ((buf[0] & 0xf0) == 0xe0) {
                buf_max = 3;
            } else if ((buf[0] & 0xf0) != 0xf0) {
                return INVALID_CHAR;
            }
        }
    }

    if (buf[0] < 0x80) {
        return buf[0];
    }

    switch (buf[0] & 0xf0) {
        case 0xc0:
        case 0xd0:
            return int24_t(((buf[0] & 0x1f) << 6) | (buf[1] & 0x3f));
        case 0xe0:
            return int24_t(((buf[0] & 0x0f) << 12) | ((buf[1] & 0x3f) << 6) | (buf[2] & 0x3f));
        case 0xf0:
            return int24_t(((buf[0] & 0x07) << 18) | ((buf[1] & 0x3f) << 12) | ((buf[2] & 0x3f) << 6) | (buf[3] & 0x3f));
        default:
            return INVALID_CHAR;
    }
}

std::vector<int24_t> parse_utf8(const std::string& s) noexcept;
