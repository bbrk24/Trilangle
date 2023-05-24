#pragma once

#include <cstdint>
#include "compat.hh"

struct int24_t {
    int32_t value : 24;

    int24_t() = default;
    constexpr int24_t(char x) noexcept : value(x) {}
    constexpr int24_t(unsigned char x) noexcept : value(x) {}
    constexpr explicit int24_t(int32_t x) noexcept : value(x) {}
    constexpr explicit int24_t(int64_t x) noexcept : value(static_cast<int32_t>(x)) {}
    constexpr explicit int24_t(size_t x) noexcept : value(static_cast<int32_t>(x)) {}
    constexpr explicit int24_t(long double x) noexcept : value(static_cast<int32_t>(x)) {}

    // Returns { false, this + other } when overflow does not occur, and { true, undefined } when overflow does occur.
    std::pair<bool, int24_t> add_with_overflow(int24_t other) const noexcept;
    // Returns { false, this - other } when overflow does not occur, and { true, undefined } when overflow does occur.
    std::pair<bool, int24_t> subtract_with_overflow(int24_t other) const noexcept;
    // Returns { false, this * other } when overflow does not occur, and { true, undefined } when overflow does occur.
    // Note: This performs divisions for the overflow check. When overflow checking is not required, do not use this
    // method.
    std::pair<bool, int24_t> multiply_with_overflow(int24_t other) const noexcept;

    constexpr operator int32_t() const noexcept { return value; }
    constexpr explicit operator uint32_t() const noexcept { return static_cast<uint32_t>(value); }
    constexpr explicit operator long double() const noexcept { return static_cast<long double>(value); }

#if PTRDIFF_MAX != INT32_MAX
    constexpr explicit operator ptrdiff_t() const noexcept {
        return static_cast<ptrdiff_t>(value);
    }
#endif

#if SIZE_MAX != UINT32_MAX
    constexpr explicit operator size_t() const noexcept {
        return static_cast<size_t>(value);
    }
#endif

    constexpr bool operator==(const int24_t& other) const noexcept {
        return this->value == other.value;
    }
    constexpr bool operator<(const int24_t& other) const noexcept {
        return this->value < other.value;
    }
    constexpr bool operator>(const int24_t& other) const noexcept {
        return this->value > other.value;
    }

    constexpr int24_t& operator++() noexcept {
        ++value;
        return *this;
    }

    constexpr int24_t& operator--() noexcept {
        --value;
        return *this;
    }

    constexpr int24_t operator-(int24_t rhs) const noexcept {
        return int24_t{ this->value - rhs.value };
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
    constexpr int24_t& operator*=(int24_t rhs) noexcept {
        value *= rhs.value;
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

constexpr int24_t INT24_MIN{ -0x800000 };
constexpr int24_t INT24_MAX{ 0x7fffff };

#define INT24_C(x) \
    int24_t { INT32_C(x) }
