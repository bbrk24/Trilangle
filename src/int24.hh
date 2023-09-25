#pragma once

#include <cstdint>
#include "compat.hh"

struct int24_t {
    int32_t value : 24;

    int24_t() = default;
    constexpr int24_t(char x) noexcept : value(x) {}
    constexpr int24_t(unsigned char x) noexcept : value(x) {}

    template<typename T>
    constexpr explicit int24_t(T x) noexcept : value(static_cast<int32_t>(x)) {}

    std::pair<bool, int24_t> add_with_overflow(int24_t other) const noexcept;
    std::pair<bool, int24_t> subtract_with_overflow(int24_t other) const noexcept;
    std::pair<bool, int24_t> multiply_with_overflow(int24_t other) const noexcept;

    constexpr operator int32_t() const noexcept { return value; }

    template<typename T>
    constexpr explicit operator T() const noexcept {
        return static_cast<T>(value);
    }

    constexpr bool operator==(const int24_t& other) const noexcept { return this->value == other.value; }
    constexpr bool operator<(const int24_t& other) const noexcept { return this->value < other.value; }
    constexpr bool operator>(const int24_t& other) const noexcept { return this->value > other.value; }

    constexpr int24_t& operator++() noexcept {
        ++value;
        return *this;
    }

    constexpr int24_t& operator--() noexcept {
        --value;
        return *this;
    }

    constexpr int24_t operator-(int24_t rhs) const noexcept { return int24_t{ this->value - rhs.value }; }

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

    constexpr int24_t operator~() const noexcept { return int24_t{ ~value }; }
    constexpr int24_t operator<<(int24_t rhs) const noexcept { return int24_t{ this->value << rhs.value }; }
};

constexpr int24_t INT24_MIN{ -0x800000 };
constexpr int24_t INT24_MAX{ 0x7fffff };

#define INT24_C(x) \
    int24_t { INT32_C(x) }
