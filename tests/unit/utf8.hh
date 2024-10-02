#pragma once

#include <cstring>
#include <output.hh>
#include <sstream>
#include <string_processing.hh>
#include "test-framework/test_framework.hh"

testgroup (utf8_parsing) {
    testcase (eof_immediately) {
        int24_t parsed = parse_unichar([]() noexcept { return EOF; });
        test_assert(parsed == INT24_C(-1), "EOF should be -1");
    }
    , testcase (ascii_string) {
        std::vector<int24_t> parsed = parse_utf8("abcd", false);
        std::vector<int24_t> expected = { 'a', 'b', 'c', 'd' };
        test_assert(parsed == expected, "ASCII characters should maintain their values");
    }
    , testcase (shebangs) {
        std::vector<int24_t> parsed1 = parse_utf8("#!/bin/trilangle\nabc", true);
        std::vector<int24_t> expected1 = { 'a', 'b', 'c' };
        test_assert(parsed1 == expected1, "Shebang should be removed");

        std::vector<int24_t> parsed2 = parse_utf8("#!@", false);
        std::vector<int24_t> expected2 = { '#', '!', '@' };
        test_assert(parsed2 == expected2, "Shebang should be kept");
    }
    , testcase (multibyte_characters) {
        std::vector<int24_t> parsed = parse_utf8(".\xC3\xA9.e\xCC\x81.\xF0\x9F\x8E\x88", false);
        std::vector<int24_t> expected = { '.', INT24_C(0x00e9), '.', 'e', INT24_C(0x0301), '.', INT24_C(0x1f388) };
        test_assert(parsed == expected);
    }
    , testcase (invalid_char) {
        std::vector<int24_t> parsed1 = parse_utf8("\xC0@", false);
        std::vector<int24_t> expected1 = { INT24_C(0xfffd) };
        test_assert(parsed1 == expected1, "Ill-formed UTF-8 should produce U+FFFD");

        std::vector<int24_t> parsed2 = parse_utf8("\xF9.", false);
        std::vector<int24_t> expected2 = { INT24_C(0xfffd), '.' };
        test_assert(parsed2 == expected2, "Characters after invalid byte should be left as-is");
    }
};

testgroup (utf8_output) {
    testcase (ascii_char) {
        std::ostringstream oss;
        print_unichar('a', oss);
        std::string result = oss.str();

        test_assert(result == "a", "ASCII character should maintain its value");
    }
    , testcase (high_byte) {
        std::ostringstream oss;
        print_unichar(INT24_C(0x00e9), oss);
        std::string result = oss.str();

        test_assert(result == "\xC3\xA9");
    }
    , testcase (multibyte_char) {
        std::ostringstream oss;
        print_unichar(INT24_C(0x1f388), oss);
        std::string result = oss.str();

        test_assert(result == "\xF0\x9F\x8E\x88");
    }
};
