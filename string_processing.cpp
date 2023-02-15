#include "string_processing.hh"

std::vector<int24_t> parse_utf8(const std::string& s) noexcept {
    auto iter = s.begin();

    std::vector<int24_t> vec;
    vec.reserve(s.size() / 4);

    do {
        vec.push_back(
            parse_unichar([&]() {
                if (iter == s.end()) {
                    return EOF;
                } else {
                    return static_cast<int>(*iter++);
                }
            })
        );
    } while (iter != s.end());

    return vec;
}
