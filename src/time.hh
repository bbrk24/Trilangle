#pragma once

#include <cfloat>
#include <chrono>
#include <limits>
#include <type_traits>
#include "int24.hh"

template<class Clock>
int24_t get_time() {
    constexpr auto ONE_DAY = std::chrono::duration_cast<typename Clock::duration>(std::chrono::hours(24));

#define TOO_SMALL(type) \
    (static_cast<unsigned long long>(INT24_MAX) + 1ULL >= (1ULL << std::numeric_limits<type>::digits) \
     || ONE_DAY.count() >= (1ULL << std::numeric_limits<type>::digits))

    using float_type = std::conditional_t<
        FLT_RADIX == 2,
        std::conditional_t<
            TOO_SMALL(small_float),
            std::conditional_t<TOO_SMALL(float), std::conditional_t<TOO_SMALL(double), long double, double>, float>,
            small_float>,
        // FLT_RADIX != 2. Hard to evaluate; play it safe.
        long double>;

#undef TOO_SMALL

    constexpr float_type TICKS_PER_UNIT =
        static_cast<float_type>(ONE_DAY.count()) / (1.0 + static_cast<float_type>(INT24_MAX));

    auto ticks = (Clock::now().time_since_epoch() % ONE_DAY).count();
    auto time = static_cast<float_type>(ticks) / TICKS_PER_UNIT;
    return static_cast<int24_t>(time);
}

template<class Clock>
int24_t get_date() {
    constexpr auto ONE_DAY = std::chrono::duration_cast<typename Clock::duration>(std::chrono::seconds(24 * 60 * 60));

    auto day = Clock::now().time_since_epoch() / ONE_DAY;
    return static_cast<int24_t>(day);
}
