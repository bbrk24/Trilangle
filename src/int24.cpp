#include "int24.hh"

using std::pair;

constexpr bool is_overflow(int32_t i) noexcept {
    return ((i << 8) >> 8) != i;
}

pair<bool, int24_t> int24_t::add_with_overflow(int24_t other) const noexcept {
    int32_t sum = this->value + other.value;
    return { is_overflow(sum), int24_t{ sum } };
}

pair<bool, int24_t> int24_t::subtract_with_overflow(int24_t other) const noexcept {
    int32_t difference = this->value - other.value;
    return { is_overflow(difference), int24_t{ difference } };
}

pair<bool, int24_t> int24_t::multiply_with_overflow(int24_t other) const noexcept {
    bool overflow;
#if ASM_ALLOWED
    int32_t lhs = this->value;
    const int32_t rhs = other.value;

    asm("\timull   %2, %1\n"
        "\tseto    %0\n"
        : "=rm"(overflow), "+r"(lhs)
        : "rm"(rhs)
        : "cc");

    return { overflow || is_overflow(lhs), int24_t{ lhs } };
#else
    if (this->value == 0 || other.value == 0) {
        return { false, INT24_C(0) };
    }

    if (this->value > 0) {
        if (other.value > 0) {
            overflow = (other.value > INT24_MAX.value / this->value);
        } else {
            overflow = (other.value < INT24_MIN.value / this->value);
        }
    } else if (other.value > 0) {
        overflow = (other.value > INT24_MIN.value / this->value);
    } else {
        overflow = (other.value < INT24_MAX.value / this->value);
    }

    return { overflow, int24_t{ this->value * other.value } };
#endif
}
