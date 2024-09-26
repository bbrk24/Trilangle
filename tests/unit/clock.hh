#pragma once

#include <cinttypes>
#include <cmath>
#include <time.hh>
#include "test-framework/test_framework.hh"

namespace {
class always_march_15_2000 {
public:
    using duration = std::chrono::milliseconds;
    using rep = duration::rep;
    using period = duration::period;
    using time_point = std::chrono::time_point<always_march_15_2000>;

    static constexpr bool is_steady = false;

    static inline time_point now() noexcept {
        // Unix time (ms) for 2000-03-15T01:23:45.680Z
        return time_point(duration(953'083'425'680));
    }
};

class end_of_first_day_1 {
public:
    using duration = std::chrono::nanoseconds;
    using rep = duration::rep;
    using period = duration::period;
    using time_point = std::chrono::time_point<end_of_first_day_1>;

    static constexpr bool is_steady = false;

    static inline time_point now() noexcept {
        // One time unit before midnight, rounded up
        rep x = static_cast<rep>(ceil(86'400'000'000'000.0 * 0x7f'ffff / 0x80'0000));
        return time_point(duration(x));
    }
};

class end_of_first_day_2 {
public:
    using duration = std::chrono::nanoseconds;
    using rep = duration::rep;
    using period = duration::period;
    using time_point = std::chrono::time_point<end_of_first_day_2>;

    static constexpr bool is_steady = false;

    static inline time_point now() noexcept {
        // a tiny amount before midnight
        return time_point(duration(86'400'000'000'000 - 1));
    }
};
}  // namespace

testgroup (datetime) {
    testcase (early_morning_time) {
        // 01:23:45.680
        // = 5025.68 seconds
        // = 487945.07 units
        int24_t time = get_time<always_march_15_2000>();
        test_assert(time == INT24_C(487945));
    }
    , testcase (date) {
        // 2000-03-15 is day 11031
        int24_t date = get_date<always_march_15_2000>();
        test_assert(date == INT24_C(11031));
    }
    , testcase (late_night_time_1) {
        int24_t time = get_time<end_of_first_day_1>();
        test_assert(time == INT24_MAX);
    }
    , testcase (late_night_time_2) {
        int24_t time = get_time<end_of_first_day_2>();
        test_assert(time == INT24_MAX);
    }
};
