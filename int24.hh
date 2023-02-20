#include <cstdint>
#include <cwchar>
#include "compat.hh"

#ifndef INT24_C
struct int24_t {
    int32_t value : 24;

    int24_t() = default;
    constexpr int24_t(char x) noexcept : value(x) {}
    constexpr int24_t(unsigned char x) noexcept : value(x) {}
    constexpr explicit int24_t(int32_t x) noexcept : value(x) {}
    MAYBE_UNUSED constexpr explicit int24_t(wint_t x) noexcept : value(x) {}

    constexpr explicit operator wchar_t() const noexcept {
        return static_cast<wchar_t>(value);
    }
    constexpr operator int32_t() const noexcept {
        return value;
    }
    constexpr explicit operator uint32_t() const noexcept {
        return static_cast<uint32_t>(value);
    }

    constexpr bool operator==(const int24_t& other) const noexcept {
        return this->value == other.value;
    }
    constexpr bool operator<(const int24_t& other) const noexcept {
        return this->value < other.value;
    }

    constexpr int24_t& operator+=(int24_t rhs) noexcept {
        value += rhs.value;
        return *this;
    }
    constexpr int24_t& operator-=(int24_t rhs) noexcept {
        value -= rhs.value;
        return *this;
    }
    constexpr int24_t& operator*=(int24_t rhs) noexcept {
        value *= rhs.value;
        return *this;
    }
    constexpr int24_t& operator%=(int24_t rhs) noexcept {
        value %= rhs.value;
        return *this;
    }
    constexpr int24_t& operator&=(int24_t rhs) noexcept {
        value &= rhs.value;
        return *this;
    }
    constexpr int24_t& operator/=(int24_t rhs) noexcept {
        value /= rhs.value;
        return *this;
    }
    constexpr int24_t& operator|=(int24_t rhs) noexcept {
        value |= rhs.value;
        return *this;
    }
    constexpr int24_t& operator^=(int24_t rhs) noexcept {
        value ^= rhs.value;
        return *this;
    }

    constexpr int24_t operator~() const noexcept {
        return int24_t{ ~value };
    }
    constexpr int24_t operator<<(int24_t rhs) const noexcept {
        return int24_t{ this->value << rhs.value };
    }
};

#define INT24_C(x) int24_t{ INT32_C(x) }
#endif
