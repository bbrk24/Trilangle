#include "input.hh"
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <tuple>

using std::cerr;
using std::endl;
using std::get;
using std::string;

constexpr size_t BUF_SIZE = 128;

constexpr const char* HELP_HEADER =
    "TRILANGLE\n\n"
    "\tTrilangle is a 2-D, stack-based esoteric programming language.\n\n"
    "Usage: %s [flags] [--] [filename]\n"
    "For full documentation, see Github: https://github.com/bbrk24/Trilangle#readme\n\n";

static constexpr const char* FLAGS_HELP =
    "Flags:\n\n"
    "\t--help           \tShow this message then exit.\n\n"
    "\t--debug, -d      \tEnter debugging mode.\n"
    "\t--show-stack, -s \tShow the stack while debugging. Requires\n"
    "\t                 \t--debug.\n"
    "\t--warnings, -w   \tShow warnings for unspecified behavior.\n"
    "\t--pipekill, -f   \tEnd the program once STDOUT is closed.\n\n"
    "\t--disassemble, -D\tOutput a pseudo-assembly representation of the\n"
    "\t                 \tcode. Incompatible with --debug, --warnings, and\n"
    "\t                 \t--pipekill.\n"
    "\t--hide-nops, -n  \tDon't include NOPs in the disassembly. Requires\n"
    "\t                 \t--disassemble.\n\n"
    "\t--expand, -e     \tSpace the program out to fit the triangle.\n"
    "\t                 \tIncompatible with all other flags.";

namespace flag_container {
static CONSTINIT_LAMBDA std::tuple<const char*, char, void (*)(flags&) NOEXCEPT_T> FLAGS[] = {
    { "debug", 'd', [](flags& f) NOEXCEPT_T { f.debug = true; } },
    { "expand", 'e', [](flags& f) NOEXCEPT_T { f.expand = true; } },
    { "warnings", 'w', [](flags& f) NOEXCEPT_T { f.warnings = true; } },
    { "pipekill", 'f', [](flags& f) NOEXCEPT_T { f.pipekill = true; } },
    { "hide-nops", 'n', [](flags& f) NOEXCEPT_T { f.hide_nops = true; } },
    { "show-stack", 's', [](flags& f) NOEXCEPT_T { f.show_stack = true; } },
    { "disassemble", 'D', [](flags& f) NOEXCEPT_T { f.disassemble = true; } },
};

[[noreturn]] static inline void invalid_flags() {
    cerr << FLAGS_HELP << endl;
    exit(EX_USAGE);
}

[[noreturn]] static inline void unrecognized_flag(NONNULL_PTR(const char) flag) {
    cerr << "Unrecognized flag: " << flag << "\n\n";
    invalid_flags();
}

static inline void set_flag(NONNULL_PTR(const char) flagname, flags& f) {
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
}  // namespace flag_container

// Read the entire contents of an istream into a string. Reads BUF_SIZE bytes at a time.
static string read_istream(std::istream& stream) {
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
                puts(FLAGS_HELP);
                exit(EXIT_SUCCESS);
            } else if (!strcmp(argv[i] + 1, "-")) {
                parse_flags = false;
                continue;
            }

            flag_container::set_flag(argv[i], f);
        } else {
            if (filename != nullptr) {
                cerr << "Please specify only one filename." << endl;
                exit(EX_USAGE);
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
            exit(EX_NOINPUT);
        }
    }
}
