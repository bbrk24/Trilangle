#pragma once

#include <chrono>
#include "int24.hh"

template<class Clock>
int24_t get_time() {
    constexpr auto ONE_DAY = std::chrono::duration_cast<typename Clock::duration>(std::chrono::seconds(24 * 60 * 60));
    constexpr auto TICKS_PER_UNIT = static_cast<long double>(ONE_DAY.count()) / static_cast<long double>(INT24_MAX);

    auto ticks = (Clock::now().time_since_epoch() % ONE_DAY).count();
    auto time = static_cast<long double>(ticks) / TICKS_PER_UNIT;
    return static_cast<int24_t>(time);
}

template<class Clock>
int24_t get_date() {
    constexpr auto ONE_DAY = std::chrono::duration_cast<typename Clock::duration>(std::chrono::seconds(24 * 60 * 60));

    auto day = Clock::now().time_since_epoch() / ONE_DAY;
    return static_cast<int24_t>(day);
}
