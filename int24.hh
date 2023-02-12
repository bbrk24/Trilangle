#include <cstdint>

#ifndef INT24_C
#include <compare>

struct int24_t {
    int32_t value : 24;

    int24_t() = default;
    constexpr int24_t(char x) noexcept : value(x) {}
    constexpr int24_t(unsigned char x) noexcept : value(x) {}
    constexpr explicit int24_t(int32_t x) noexcept : value(x) {}
    constexpr explicit int24_t(wint_t x) noexcept : value(x) {}

    constexpr explicit operator char() const noexcept {
        return value;
    }

    auto operator<=>(const int24_t&) const = default;
};

#define INT24_C(x) int24_t{ INT32_C(x) }
#endif
