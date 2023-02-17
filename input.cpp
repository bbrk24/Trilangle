#include "input.hh"
#include "string_processing.hh"
#include <iostream>
#include <fstream>
#include <cstring>
#include <tuple>

using std::string;
using std::get;

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
"\t--pipekill, -f  \tEnd the program once STDOUT is closed\n"
;

namespace flag_container {
    static CONSTINIT_LAMBDA const std::tuple<const char*, char, void(*)(flags&) NOEXCEPT_T> FLAGS[] = {
        { "debug", 'd', [](flags& f) NOEXCEPT_T { f.debug = true; } },
        { "show-stack", 's', [](flags& f) NOEXCEPT_T { f.show_stack = true; } },
        { "warnings", 'w', [](flags& f) NOEXCEPT_T { f.warnings = true; } },
        { "pipekill", 'f', [](flags& f) NOEXCEPT_T { f.pipekill = true; } },
    };

    [[noreturn]] static inline void unrecognized_flag(const char* flag) {
        std::cerr << "Unrecognized flag: " << flag << std::endl;
        exit(1);
    }
    
    static inline void set_flag(const char* flagname, flags& f) {
        if (flagname[1] == '-') {
            for (const auto& t : FLAGS) {
                if (!strcmp(get<0>(t), flagname + 2)) {
                    get<2>(t)(f);
                    return;
                }
            }
            unrecognized_flag(flagname);
        } else {
            for (string_index i = 1; flagname[i] != '\0'; ++i) {
                bool known_flag = false;

                for (const auto& t : FLAGS) {
                    if (get<1>(t) == flagname[i]) {
                        get<2>(t)(f);
                        known_flag = true;
                        break;
                    }
                }

                if (!known_flag) {
                    unrecognized_flag(flagname);
                }
            }
        }
    }
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

string parse_args(int argc, char** argv, flags& f) {
    char* filename = nullptr;

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (!strcmp(argv[i] + 1, "-help")) {
                printf(HELP, argv[0]);
                exit(0);
            }

            flag_container::set_flag(argv[i], f);
        } else {
            if (filename != nullptr) {
                std::cerr << "Error: please specify only one filename." << std::endl;
                exit(1);
            }

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
