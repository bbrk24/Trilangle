#include "int24.hh"

using std::pair;

pair<bool, int24_t> int24_t::add_with_overflow(int24_t other) const noexcept {
    bool overflow;
    // I tried some clever bitwise magic before, but it didn't work.
    if (this->value > 0) {
        overflow = other.value > INT24_MAX.value - this->value;
    } else {
        overflow = other.value < INT24_MIN.value - this->value;
    }

    return { overflow, int24_t{ this->value + other.value } };
}

pair<bool, int24_t> int24_t::subtract_with_overflow(int24_t other) const noexcept {
    bool overflow;
    if (other.value > 0) {
        overflow = other.value > this->value - INT24_MIN.value;
    } else {
        overflow = other.value < this->value - INT24_MAX.value;
    }

    return { overflow, int24_t{ this->value - other.value } };
}

pair<bool, int24_t> int24_t::multiply_with_overflow(int24_t other) const noexcept {
    bool overflow;

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
}
