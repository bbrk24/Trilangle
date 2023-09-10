#pragma once

#include <program_walker.hh>
#include "test-framework/test_framework.hh"

namespace {
// Thank you clang-format very cool
#define TEST_ITEM(dir, prog) \
    { \
#dir, { direction::dir, program(prog) } \
    }

// Each test program consists of every digit 0-9 exactly once, in the order they are hit if the IP is traveling in the
// given direction from the top corner.
std::initializer_list<std::pair<const char*, std::pair<direction, program>>> test_programs = {
    TEST_ITEM(southwest, "0142573689"),
    TEST_ITEM(west, "0215439876"),
    TEST_ITEM(northwest, "0395286417"),
    TEST_ITEM(northeast, "0968537421"),
    TEST_ITEM(east, "0895671234"),
    TEST_ITEM(southeast, "0715824693"),
};

#undef TEST_ITEM
}  // namespace

test_iter(ip_advance, test_programs, input) {
    program& p = input.second;
    program_walker::instruction_pointer ip{ { SIZE_C(0), SIZE_C(0) }, input.first };

    for (int24_t i = '0'; i <= '9'; ++i) {
        test_assert(p.at(ip.coords.first, ip.coords.second) == i);
        program_walker::advance(ip, p.side_length());
    }
    test_assert(ip.coords.first == 0 && ip.coords.second == 0);
};

// TODO: Add tests for branch instructions
