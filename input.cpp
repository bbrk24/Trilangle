#include "input.hh"
#include "string_processing.hh"
#include <iostream>
#include <fstream>
#include <cstring>
#include <tuple>

using std::string;
using std::get;
using std::cerr;
using std::endl;

constexpr size_t BUF_SIZE = 256;

constexpr const char* HELP_HEADER = "TRILANGLE\n\n"
"\tTrilangle is a 2-D, stack-based esoteric programming language.\n\n"
"Usage: %s [flags] [--] [filename]\n"
"For full documentation, see Github: https://github.com/bbrk24/Trilangle#readme\n\n"
;

static constexpr const char* FLAGS_HELP = "Flags:\n\n"
"\t--help           \tShow this message then exit.\n\n"
"\t--debug, -d      \tEnter debugging mode.\n"
"\t--show-stack, -s \tShow the stack while debugging. Requires --debug.\n"
"\t--warnings, -w   \tShow warnings for unspecified behavior.\n"
"\t--pipekill, -f   \tEnd the program once STDOUT is closed.\n\n"
"\t--disassemble, -D\tOutput a pseudo-assembly representation of the\n"
"\t                 \tcode. Incompatible with --debug, --warnings, and\n"
"\t                 \t--pipekill.\n"
"\t--hide-nops, -n  \tDon't include NOPs in the disassembly. Requires\n"
"\t                 \t--disassemble."
;

namespace flag_container {
    static CONSTINIT_LAMBDA const std::tuple<const char*, char, void(*)(flags&) NOEXCEPT_T> FLAGS[] = {
        { "debug", 'd', [](flags& f) NOEXCEPT_T { f.debug = true; } },
        { "show-stack", 's', [](flags& f) NOEXCEPT_T { f.show_stack = true; } },
        { "warnings", 'w', [](flags& f) NOEXCEPT_T { f.warnings = true; } },
        { "pipekill", 'f', [](flags& f) NOEXCEPT_T { f.pipekill = true; } },
        { "disassemble", 'D', [](flags& f) NOEXCEPT_T { f.disassemble = true; } },
        { "hide-nops", 'n', [](flags& f) NOEXCEPT_T { f.hide_nops = true; } },
    };

    [[noreturn]] static inline void invalid_flags() {
        cerr << FLAGS_HELP << endl;
        exit(1);
    }

    [[noreturn]] static inline void unrecognized_flag(const char* flag) {
        cerr << "Unrecognized flag: " << flag << "\n\n";
        invalid_flags();
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

string parse_args(int argc, const char** argv, flags& f) {
    const char* filename = nullptr;

    bool parse_flags = true;
    for (int i = 1; i < argc; ++i) {
        if (parse_flags && argv[i][0] == '-') {
            if (!strcmp(argv[i] + 1, "-help")) {
                printf(HELP_HEADER, argv[0]);
                std::cout << FLAGS_HELP << endl;
                exit(0);
            } else if (!strcmp(argv[i] + 1, "-")) {
                parse_flags = false;
                continue;
            }

            flag_container::set_flag(argv[i], f);
        } else {
            if (filename != nullptr) {
                cerr << "Please specify only one filename." << endl;
                exit(1);
            }

            filename = argv[i];
        }
    }

    if (!f.is_valid()) {
        cerr << "Invalid combination of flags.\n\n";
        flag_container::invalid_flags();
    }

    if (filename == nullptr) {
        return read_istream(std::cin);
    } else {
        std::ifstream f_input(filename, std::ios_base::in);

        if (f_input.is_open()) {
            return read_istream(f_input);
        } else {
            cerr << "File could not be opened for reading: " << filename << endl;
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
