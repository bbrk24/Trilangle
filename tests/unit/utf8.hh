#pragma once

#include <cstring>
#include <output.hh>
#include <sstream>
#include <string_processing.hh>
#include "test-framework/test_framework.hh"

using std::ostringstream;
using std::string;
using std::vector;

testgroup (utf8_parsing) {
    testcase (eof_immediately) {
        int24_t parsed = parse_unichar([]() NOEXCEPT_T { return EOF; });
        test_assert(parsed == (int24_t)-1, "EOF should be -1");
    }
    , testcase (ascii_string) {
        vector<int24_t> parsed = parse_utf8("abcd", false);
        vector<int24_t> expected = { 'a', 'b', 'c', 'd' };
        test_assert(parsed == expected, "ASCII characters should maintain their values");
    }
    , testcase (shebangs) {
        vector<int24_t> parsed1 = parse_utf8("#!/bin/trilangle\nabc", true);
        vector<int24_t> expected1 = { 'a', 'b', 'c' };
        test_assert(parsed1 == expected1, "Shebang should be removed");

        vector<int24_t> parsed2 = parse_utf8("#!@", false);
        vector<int24_t> expected2 = { '#', '!', '@' };
        test_assert(parsed2 == expected2, "Shebang should be kept");
    }
    , testcase (multibyte_characters) {
        vector<int24_t> parsed = parse_utf8(".\xC3\xA9.e\xCC\x81.\xF0\x9F\x8E\x88", false);
        vector<int24_t> expected = { '.', INT24_C(0x00e9), '.', 'e', INT24_C(0x0301), '.', INT24_C(0x1f388) };
        test_assert(parsed == expected);
    }
};

testgroup (utf8_output) {
    testcase (ascii_char) {
        ostringstream oss;
        print_unichar('a', oss);
        string result = oss.str();

        test_assert(result == "a", "ASCII character should maintain its value");
    }
    , testcase (high_byte) {
        ostringstream oss;
        print_unichar(INT24_C(0x00e9), oss);
        string result = oss.str();

        test_assert(result == "\xC3\xA9");
    }
    , testcase (multibyte_char) {
        ostringstream oss;
        print_unichar(INT24_C(0x1f388), oss);
        string result = oss.str();

        test_assert(result == "\xF0\x9F\x8E\x88");
    }
};
