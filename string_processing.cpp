#include "string_processing.hh"

std::vector<int24_t> parse_utf8(const std::string& s, bool skip_shebang) noexcept {
    auto iter = s.begin();

    std::vector<int24_t> vec;
    vec.reserve(s.size() / 4);

    if (skip_shebang && s.size() > 2 && s[0] == '#' && s[1] == '!') {
        while (*iter++ != '\n');
    }

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
