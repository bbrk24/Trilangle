#pragma once

#include <cstdint>
#include <cwchar>
#include "compat.hh"

struct int24_t {
    int32_t value : 24;

    int24_t() = default;
    constexpr int24_t(char x) noexcept : value(x) {}
    constexpr int24_t(unsigned char x) noexcept : value(x) {}
    constexpr explicit int24_t(int32_t x) noexcept : value(x) {}
    constexpr explicit int24_t(int64_t x) noexcept : value(static_cast<int32_t>(x)) {}
    constexpr explicit int24_t(size_t x) noexcept : value(static_cast<int32_t>(x)) {}
    MAYBE_UNUSED constexpr explicit int24_t(wint_t x) noexcept : value(static_cast<int32_t>(x)) {}

    constexpr operator int32_t() const noexcept { return value; }
    constexpr explicit operator uint32_t() const noexcept { return static_cast<uint32_t>(value); }
    constexpr explicit operator int64_t() const noexcept { return static_cast<int64_t>(value); }
    MAYBE_UNUSED constexpr explicit operator int16_t() const noexcept { return static_cast<int16_t>(value); }

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

    constexpr int24_t& operator++() noexcept {
        ++value;
        return *this;
    }

    constexpr int24_t& operator--() noexcept {
        --value;
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

#define INT24_C(x) \
    int24_t { INT32_C(x) }
