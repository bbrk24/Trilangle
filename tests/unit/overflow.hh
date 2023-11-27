#pragma once

#include <int24.hh>
#include "test-framework/test_framework.hh"

testgroup (overflow) {
    testcase (mul_no_overflow) {
        int24_t first = INT24_C(3);
        int24_t second = INT24_C(-2);
        auto result = first.multiply_with_overflow(second);
        test_assert(!result.first && result.second == INT24_C(-6));
    }
    , testcase (mul_negative_min) {
        int24_t minus_one = INT24_C(-1);
        auto result = minus_one.multiply_with_overflow(INT24_MIN);
        test_assert(result.first && result.second == INT24_MIN);
    }
    , testcase (mul_very_overflow) {
        int24_t huge_enough = INT24_C(0x400000);
        auto result = huge_enough.multiply_with_overflow(huge_enough);
        test_assert(result.first && result.second == INT24_C(0));
    }
    , testcase (mul_unsigned) {
        int24_t first = INT24_C(0x700000);
        int24_t second = INT24_C(2);
        auto result = first.multiply_with_overflow(second);
        test_assert(!result.first && result.second == INT24_C(0xe00000));
    }
    , testcase (inc_max) {
        int24_t x = INT24_MAX;
        ++x;
        test_assert(x == INT24_MIN);
    }
    , testcase (dec_min) {
        int24_t x = INT24_MIN;
        --x;
        test_assert(x == INT24_MAX);
    }
};
