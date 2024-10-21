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

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#if !defined(VERSION) && !defined(__EMSCRIPTEN__)
#pragma message("VERSION not set. Define it to enable the --version flag.")
#endif

constexpr size_t BUF_SIZE = 128;

constexpr const char* HELP_HEADER =
    "TRILANGLE\n\n"
    "\tTrilangle is a 2-D, stack-based esoteric programming language.\n\n"
    "Usage: %s [flags] [--] [filename]\n"
    "For full documentation, see Github: https://github.com/bbrk24/Trilangle#readme\n\n";

static constexpr const char* FLAGS_HELP =
    "Flags:\n\n"
    "\t--help           \tShow this message then exit.\n"
#ifdef VERSION
    "\t--version        \tPrint the version number then exit.\n"
#endif
    "\n"
    "\t--debug, -d      \tEnter debugging mode.\n"
    "\t--show-stack, -s \tShow the stack while debugging. Requires\n"
    "\t                 \t--debug.\n"
    "\t--warnings, -w   \tShow warnings for unspecified behavior.\n"
    "\t--pipekill, -f   \tEnd the program once STDOUT is closed.\n"
    "\t--ascii, -a      \tAssume all I/O is ASCII-only. Incompatible with\n"
    "\t                 \t--disassemble and --expand.\n\n"
    "\t--disassemble, -D\tOutput a pseudo-assembly representation of the\n"
    "\t                 \tcode. Incompatible with --debug, --warnings, and\n"
    "\t                 \t--pipekill.\n"
    "\t--hide-nops, -n  \tDon't include NOPs in the disassembly. Requires\n"
    "\t                 \t--disassemble.\n"
    "\t--compile, -c    \tOutput C code for the program. Doesn't support all\n"
    "\t                 \toperations. Incompatible with all other flags except\n"
    "\t                 \t--null.\n"
    "\t--assembly, -A   \tTake the input in the pseudo-assembly format.\n"
    "\t                 \tIncompatible with --disassemble and --compile.\n\n"
    "\t--expand, -e     \tSpace the program out to fit the triangle.\n"
    "\t                 \tIncompatible with all other flags except --null.\n"
    "\t--null, -z       \tRead the program until null terminator instead of EOF.";

namespace flag_container {
static constexpr std::tuple<NONNULL_PTR(const char), char, void (*)(flags&) noexcept> FLAGS[] = {
    { "null", 'z', [](flags& f) noexcept { f.null_terminated = true; } },
    { "debug", 'd', [](flags& f) noexcept { f.debug = true; } },
    { "ascii", 'a', [](flags& f) noexcept { f.assume_ascii = true; } },
    { "expand", 'e', [](flags& f) noexcept { f.expand = true; } },
    { "compile", 'c', [](flags& f) noexcept { f.compile = true; } },
    { "warnings", 'w', [](flags& f) noexcept { f.warnings = true; } },
    { "pipekill", 'f', [](flags& f) noexcept { f.pipekill = true; } },
    { "assembly", 'A', [](flags& f) noexcept { f.assembly = true; } },
    { "hide-nops", 'n', [](flags& f) noexcept { f.hide_nops = true; } },
    { "show-stack", 's', [](flags& f) noexcept { f.show_stack = true; } },
    { "disassemble", 'D', [](flags& f) noexcept { f.disassemble = true; } },
};

[[noreturn]] static inline void invalid_flags() {
    cerr << FLAGS_HELP << endl;
    exit(EX_USAGE);
}

[[noreturn]] static inline void unrecognized_flag(CONST_C_STR flag) {
    cerr << "Unrecognized flag: " << flag << "\n\n";
    invalid_flags();
}

static inline void set_flag(CONST_C_STR flag_name, flags& f) {
    if (flag_name[1] == '-') {
        for (const auto& t : FLAGS) {
            if (!strcmp(get<0>(t), flag_name + 2)) {
                get<2>(t)(f);
                return;
            }
        }
        unrecognized_flag(flag_name);
    } else {
        for (string_index i = 1; flag_name[i] != '\0'; ++i) {
            bool known_flag = false;

            for (const auto& t : FLAGS) {
                if (get<1>(t) == flag_name[i]) {
                    get<2>(t)(f);
                    known_flag = true;
                    break;
                }
            }

            if (!known_flag) {
                unrecognized_flag(flag_name);
            }
        }
    }
}
}  // namespace flag_container

namespace {
// Read the entire contents of an istream into a string. Reads BUF_SIZE bytes at a time.
// If null_terminated is true, only read until the first null byte, not EOF.
string read_istream(std::istream& stream, bool null_terminated) {
    string retval;

    if (null_terminated) {
        std::getline(stream, retval, '\0');
    } else {
        char buf[BUF_SIZE];

        while (stream.read(buf, BUF_SIZE)) {
            retval.append(buf, BUF_SIZE);
        }
        retval.append(buf, static_cast<size_t>(stream.gcount()));
    }

    return retval;
}
}  // namespace

string parse_args(int argc, _In_reads_z_(argc) const char** argv, flags& f) {
    const char* filename = nullptr;

    bool parse_flags = true;
    for (int i = 1; i < argc; ++i) {
        if (parse_flags && argv[i][0] == '-') {
            if (!strcmp(argv[i] + 1, "-help")) {
                printf(HELP_HEADER, argv[0]);
                puts(FLAGS_HELP);
                exit(EXIT_SUCCESS);
            }
#ifdef VERSION
            if (!strcmp(argv[i] + 1, "-version")) {
                puts("Trilangle version " STRINGIFY(VERSION));
                exit(EXIT_SUCCESS);
            }
#endif
            if (!strcmp(argv[i] + 1, "-")) {
                parse_flags = false;
                continue;
            }

            if (argv[i][1] == '\0') {
                cerr << "'-' by itself is not a valid flag.\nTo reference a file called '-', use './-' or '-- -'. To "
                        "read from stdin, pass no\nfilename at all."
                     << endl;
                exit(EX_USAGE);
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
        return read_istream(std::cin, f.null_terminated);
    } else {
        std::ifstream f_input(filename, std::ios_base::in);

        if (f_input.is_open()) {
            return read_istream(f_input, f.null_terminated);
        } else {
            cerr << "File could not be opened for reading: " << filename << endl;
            exit(EX_NOINPUT);
        }
    }
}
