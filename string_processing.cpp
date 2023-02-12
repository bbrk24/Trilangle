#include "string_processing.hh"

using std::vector;

vector<int24_t> parse_utf8(const std::string& s) noexcept {
    auto iter = s.begin();

    vector<int24_t> vec;
    vec.reserve(s.size() / 4);

    do {
        vec.push_back(
            parse_unichar([&]() {
                if (iter == s.end()) {
                    return EOF;
                } else {
                    return (int)*iter++;
                }
            })
        );
    } while (iter != s.end());

    return vec;
}
