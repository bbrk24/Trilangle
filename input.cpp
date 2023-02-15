#include "input.hh"
#include "string_processing.hh"
#include <iostream>
#include <fstream>
#include <cstring>
#include <unordered_map>

using std::string;

constexpr size_t BUF_SIZE = 256;

static constexpr const char* HELP = "TRILANGLE\n\n"
"\tTrilangle is an esoteric programming language inspired by Hexagony.\n\n"
"Usage: %s <filename> [flags]\n"
"For full documentation, see Github: https://github.com/bbrk24/Trilangle#readme\n\n"
"Flags:\n"
"\t--help          \tShow this message\n"
"\t--debug, -d     \tEnter debugging mode\n"
"\t--show-stack, -s\tShow the stack while debugging\n"
"\t--warnings, -w  \tShow warnings for unspecified behavior\n"
;

static const std::unordered_map<char, void(*)(flags&)> FLAGS_MAP = {
    { 'd', [](flags& f) { f.debug = true; } },
    { 's', [](flags& f) { f.show_stack = true; } },
    { 'w', [](flags& f) { f.warnings = true; } },
};

// Read the entire contents of an istream into a string. Reads BUF_SIZE bytes at a time.
static inline string read_istream(std::istream& stream) {
    string retval;
    char buf[BUF_SIZE];

    while (stream.read(buf, BUF_SIZE)) {
        retval.append(buf, BUF_SIZE);
    }
    retval.append(buf, static_cast<size_t>(stream.gcount()));

    return retval;
}

[[noreturn]] static inline void unrecognized_flag(const char* flag) {
    std::cerr << "Unrecognized flag: " << flag << std::endl;
    exit(1);
}

string parse_args(int argc, char** argv, flags& f) {
    char* filename = nullptr;

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == '-') {
                if (!strcmp(argv[i] + 2, "debug")) {
                    FLAGS_MAP.at('d')(f);
                } else if (!strcmp(argv[i] + 2, "show-stack")) {
                    FLAGS_MAP.at('s')(f);
                } else if (!strcmp(argv[i] + 2, "warnings")) {
                    FLAGS_MAP.at('w')(f);
                } else if (!strcmp(argv[i] + 2, "help")) {
                    printf(HELP, argv[0]);
                    exit(0);
                } else {
                    unrecognized_flag(argv[i]);
                }
            } else {
                for (uintptr_t j = 1; argv[i][j] != '\0'; ++j) {
                    try {
                        FLAGS_MAP.at(argv[i][j])(f);
                    } catch (std::out_of_range) {
                        unrecognized_flag(argv[i]);
                    }
                }
            }
        } else {
            filename = argv[i];
        }
    }

    if (filename == nullptr) {
        return read_istream(std::cin);
    } else {
        std::ifstream f_input(filename, std::ios_base::in);

        if (f_input.is_open()) {
            return read_istream(f_input);
        } else {
            std::cerr << "File could not be opened for reading: " << filename << std::endl;
            exit(1);
        }
    }
}

int24_t getunichar() noexcept {
#if WINT_MAX >= 0x10ffff && (WINT_MAX - WINT_MIN) > 0x10ffff
    wint_t c = getwchar();

    if (c == WEOF) {
        return INT24_C(-1);
    }

    return static_cast<int24_t>(c);
#else
    return parse_unichar(getchar);
#endif
}
