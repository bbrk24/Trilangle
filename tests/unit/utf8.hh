#include <string_processing.hh>
#include "test-framework/test_framework.hh"
#include "vector_compare.hh"

using std::vector;

testgroup (utf8) {
    testcase (eof_immediately) {
        int24_t parsed = parse_unichar([]() NOEXCEPT_T { return EOF; });
        test_assert(parsed == (int24_t)-1, "EOF should be -1");
    }
    , testcase (ascii_string) {
        vector<int24_t> parsed = parse_utf8("abcd", false);
        vector<int24_t> expected = { 'a', 'b', 'c', 'd' };
        test_assert(vector_equal(parsed, expected), "ASCII characters should maintain their values");
    }
    , testcase (shebangs) {
        vector<int24_t> parsed1 = parse_utf8("#!/bin/trilangle\nabc", true);
        vector<int24_t> expected1 = { 'a', 'b', 'c' };
        test_assert(vector_equal(parsed1, expected1), "Shebang should be removed");

        vector<int24_t> parsed2 = parse_utf8("#!@", false);
        vector<int24_t> expected2 = { '#', '!', '@' };
        test_assert(vector_equal(parsed2, expected2), "Shebang should be kept");
    }
    , testcase (multibyte_characters) {
        vector<int24_t> parsed = parse_utf8(".\xC3\xA9.e\xCC\x81.\xF0\x9F\x8E\x88", false);
        vector<int24_t> expected = { '.', INT24_C(0x00e9), '.', 'e', INT24_C(0x0301), '.', INT24_C(0x1f388) };
        test_assert(vector_equal(parsed, expected));
    }
};
