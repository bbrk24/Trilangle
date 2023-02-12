#pragma once

#include "int24.hh"
#include <type_traits>
#include <vector>
#include <string>
#include <concepts>

// Given a function that yields one "character" at a time, parse the incoming bytestream as UTF-8.
template<typename T, typename FuncType>
int24_t parse_unichar(FuncType getbyte) {
    unsigned char buf[4];

    for (size_t i = 0; i < 4; ++i) {
        auto c = getbyte();
        if constexpr (std::is_signed_v<T> && sizeof (T) > 1) {
            if (c == EOF) {
                return INT24_C(-1);
            }
        }

        buf[i] = (unsigned char)c;
        if (buf[i] < 0x80) {
            break;
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
        return INT24_C(0xfffd);
    }
}

std::vector<int24_t> parse_utf8(const std::string& s) noexcept;
