#include "input.hh"
#include "string_processing.hh"
#include <iostream>
#include <fstream>
#include <cstring>

using std::cerr;
using std::endl;
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
;

// Read the entire contents of an istream into a string. Reads BUF_SIZE bytes at a time.
static inline string read_istream(std::istream& stream) {
    string retval;
    char buf[BUF_SIZE];

    while (stream.read(buf, BUF_SIZE)) {
        retval.append(buf, BUF_SIZE);
    }
    retval.append(buf, stream.gcount());

    return retval;
}

string parse_args(int argc, char** argv, flags& f) {
    char* filename = nullptr;

    for (int i = 1; i < argc; ++i) {
        if (argv[i][0] == '-') {
            if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug")) {
                f.debug = 1;
            } else if (!strcmp(argv[i], "-s") || !strcmp(argv[i], "--show-stack")) {
                f.show_stack = 1;
            } else if (!strcmp(argv[i], "-sd") || !strcmp(argv[i], "-ds")) {
                f.debug = 1;
                f.show_stack = 1;
            } else if (!strcmp(argv[i], "--help")) {
                printf(HELP, argv[0]);
                exit(0);
            } else {
                cerr << "Unrecognized flag: " << argv[i] << endl;
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

    return int24_t(c);
#else
    return parse_unichar(getchar);
#endif
}
