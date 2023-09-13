#pragma once

#include <program_walker.hh>
#include <tuple>
#include "test-framework/test_framework.hh"

using std::pair;

namespace {
// Thank you clang-format very cool
#define TEST_ITEM(dir, prog) \
    { \
#dir, { direction::dir, program(prog) } \
    }

// Each test program consists of every digit 0-9 exactly once, in the order they are hit if the IP is traveling in the
// given direction from the top corner.
std::initializer_list<pair<const char*, pair<direction, program>>> test_programs = {
    TEST_ITEM(southwest, "0142573689"),
    TEST_ITEM(west, "0215439876"),
    TEST_ITEM(northwest, "0395286417"),
    TEST_ITEM(northeast, "0968537421"),
    TEST_ITEM(east, "0895671234"),
    TEST_ITEM(southeast, "0715824693"),
};

#undef TEST_ITEM

#define LR_PAIR(left, left_branch, right, right_branch) \
    { #right "_left", { direction::right, right_branch, true, direction::left } }, { \
#left "_right", { direction::left, left_branch, false, direction::right } \
    }

// Each item is a 4-tuple of starting direction, instruction, should go left, ending direction.
std::initializer_list<pair<const char*, std::tuple<direction, int24_t, bool, direction>>> branch_tests = {
    LR_PAIR(southwest, '7', west, '>'), LR_PAIR(west, '>', northwest, 'v'), LR_PAIR(northwest, 'v', northeast, 'L'),
    LR_PAIR(northeast, 'L', east, '<'), LR_PAIR(east, '<', southeast, '^'), LR_PAIR(southeast, '^', southwest, '7'),
};

#undef LR_PAIR
}  // namespace

test_iter(ip_advance, test_programs, input) {
    const program& p = input.second;
    program_walker::instruction_pointer ip{ { SIZE_C(0), SIZE_C(0) }, input.first };

    for (int24_t i = '0'; i <= '9'; ++i) {
        test_assert(p.at(ip.coords.first, ip.coords.second) == i);
        program_walker::advance(ip, p.side_length());
    }
    test_assert(ip.coords.first == 0 && ip.coords.second == 0);
};

test_iter(ip_branch, branch_tests, input) {
    direction dir = std::get<0>(input);
    program_walker::branch(dir, std::get<1>(input), [&]() NOEXCEPT_T { return std::get<2>(input); });
    test_assert(dir == std::get<3>(input));
};

// TODO: add mirror tests
