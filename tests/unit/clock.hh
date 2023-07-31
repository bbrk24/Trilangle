#pragma once

#include <cinttypes>
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
        return time_point(duration(953083425680));
    }
};
}  // namespace

testgroup (datetime) {
    testcase (time) {
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
};
