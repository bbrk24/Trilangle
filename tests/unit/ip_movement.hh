#pragma once

#include <program_walker.hh>
#include "test-framework/test_framework.hh"

testgroup (ip_advance) {
    testcase (sample_program) {
        /*
         * From the README:
         *    0
         *   1 4
         *  2 5 7
         * 3 6 8 9
         */
        program p("0142573689");
        program_walker::instruction_pointer ip{ { SIZE_C(0), SIZE_C(0) }, direction::southwest };

        for (int24_t i = '0'; i <= '9'; ++i) {
            test_assert(p.at(ip.coords.first, ip.coords.second) == i);
            program_walker::advance(ip, p.side_length());
        }

        test_assert(ip.coords.first == 0 && ip.coords.second == 0);
    }
};

// TODO: Add tests for branch instructions
