#include "int24.hh"

using std::pair;

pair<bool, int24_t> int24_t::add_with_overflow(int24_t other) const noexcept {
    int32_t edi = this->value + other.value;
    return { static_cast<bool>(edi & 0x800000), int24_t{ (edi & 0x7fffff) | -(edi & 0x800000) } };
}

pair<bool, int24_t> int24_t::subtract_with_overflow(int24_t other) const noexcept {
    int32_t edi = this->value - other.value;
    return { static_cast<bool>(edi & 0x800000), int24_t{ (edi & 0x7fffff) | -(edi & 0x800000) } };
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

    overflow = overflow || static_cast<bool>(lhs & 0x800000);
    lhs = (lhs & 0x7fffff) | -(lhs & 0x800000);

    return { overflow, int24_t{ lhs } };
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
