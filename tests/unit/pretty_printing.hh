#pragma once

#include <program.hh>
#include <sstream>
#include "test-framework/test_framework.hh"

testgroup (expand_program) {
    testcase (single_char) {
        program p(".");
        std::ostringstream oss;
        oss << p;
        std::string result = oss.str();
        test_assert(result == ".\n", "Single character should not have spacing but all programs end in a newline");
    }
    , testcase (primes) {
        program p("<'?<#2%._zS<.>(/.,)2-.^\\_/!@.)@");
        std::ostringstream oss;
        oss << p;
        std::string result = oss.str();
        test_assert(result == &R"#(
       <
      ' ?
     < # 2
    % . _ z
   S < . > (
  / . , ) 2 -
 . ^ \ _ / ! @
. ) @ . . . . .
)#"[1]);
    }
};
