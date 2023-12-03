#include "int24.hh"

using std::pair;

constexpr bool is_overflow(int32_t i) noexcept {
    return i < -0x0080'0000 || i > 0x00ff'ffff;
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
    if ((*this == INT24_MIN && other == INT24_C(-1)) || (*this == INT24_C(-1) && other == INT24_MIN)) UNLIKELY {
        return { true, INT24_MIN };
    }
    auto lhs = static_cast<int64_t>(*this), rhs = static_cast<int64_t>(other);
    int64_t result = lhs * rhs;
    return { result < -0x0080'0000 || result > 0x00ff'ffff, static_cast<int24_t>(result) };
}
