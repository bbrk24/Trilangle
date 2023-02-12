#include "input.hh"
#include <iostream>
#include <fstream>
#include <cstdio>

using std::cerr;
using std::endl;
using std::string;

constexpr size_t BUF_SIZE = 256;

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

string readfile(int argc, char** argv) {
    switch (argc) {
        case 1:
            return read_istream(std::cin);
        case 2: {
            std::ifstream f_input(argv[1], std::ios_base::in);

            if (f_input.is_open()) {
                return read_istream(f_input);
            } else {
                cerr << "File could not be opened for reading: " << argv[1] << endl;
                exit(1);
            }
        }
        default:
            cerr << "Too many arguments." << endl;
            exit(1);
    }
}

int24_t getunichar() noexcept {
#if WINT_MAX >= 0x10ffff && (WINT_MAX - WINT_MIN) > 0x10ffff
    wint_t c = getwchar();

    if (c == WEOF) {
        return INT24_C(-1);
    }

    return { c };
#else
    unsigned char buf[4];

    for (size_t i = 0; i < 4; ++i) {
        int c = getchar();
        if (c == EOF) {
            return INT24_C(-1);
        }
        buf[i] = c & 0xff;
        if (buf[i] < 0x80) {
            break;
        }
    }

    if (buf[0] < 0x80) {
        return buf[0];
    }

    switch (buf[0] & 0xf0) {
        case 0xc0:
        case 0xd0:
            return int24_t(((buf[0] & 0x1f) << 6) | (buf[1] & 0x3f));
        case 0xe0:
            return int24_t(((buf[0] & 0x0f) << 12) | ((buf[1] & 0x3f) << 6) | (buf[2] & 0x3f));
        case 0xf0:
            return int24_t(((buf[0] & 0x07) << 18) | ((buf[1] & 0x3f) << 12) | ((buf[2] & 0x3f) << 6) | (buf[3] & 0x3f));
        default:
            return INT24_C(0xfffd);
    }
#endif
}
