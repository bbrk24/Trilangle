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
    // The 64-bit version may be less efficient on some archs, e.g. one that can do 32-bit multiplication and then check
    // an overflow flag. So, use __builtin_mul_overflow when it's available, and fall back to 64-bit multiplication for
    // MSVC.
#if REALLY_MSVC
    auto lhs = static_cast<int64_t>(*this), rhs = static_cast<int64_t>(other);
    int64_t result = lhs * rhs;
    return { result < static_cast<int64_t>(INT24_MIN) || result > static_cast<int64_t>(INT24_MAX),
             static_cast<int24_t>(result) };
#else
    int32_t result;
    bool overflow = __builtin_mul_overflow(this->value, other.value, &result);
    return { overflow || is_overflow(result), int24_t{ result } };
#endif
}
