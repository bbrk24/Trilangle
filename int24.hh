#include <cstdint>
#include <compare>
#include <cwchar>

#ifndef INT24_C
struct int24_t {
    int32_t value : 24;

    int24_t() = default;
    constexpr int24_t(char x) noexcept : value(x) {}
    constexpr int24_t(unsigned char x) noexcept : value(x) {}
    constexpr explicit int24_t(int32_t x) noexcept : value(x) {}
    constexpr explicit int24_t(wint_t x) noexcept : value(x) {}

    constexpr explicit operator wchar_t() const noexcept {
        return (wchar_t)value;
    }
    constexpr operator int32_t() const noexcept {
        return value;
    }

    auto operator<=>(const int24_t&) const = default;

    int24_t& operator+=(int24_t rhs) noexcept {
        value += rhs.value;
        return *this;
    }
    int24_t& operator-=(int24_t rhs) noexcept {
        value -= rhs.value;
        return *this;
    }
    int24_t& operator*=(int24_t rhs) noexcept {
        value *= rhs.value;
        return *this;
    }
    int24_t& operator%=(int24_t rhs) noexcept {
        value %= rhs.value;
        return *this;
    }
    int24_t& operator&=(int24_t rhs) noexcept {
        value &= rhs.value;
        return *this;
    }
    int24_t& operator/=(int24_t rhs) noexcept {
        value /= rhs.value;
        return *this;
    }

    int24_t operator~() const noexcept {
        return int24_t{ ~value };
    }
};

#define INT24_C(x) int24_t{ INT32_C(x) }
#endif
