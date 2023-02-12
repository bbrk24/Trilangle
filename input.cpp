#include "input.hh"
#include <iostream>
#include <cstdlib>
#include <fstream>
#include <iterator>

using std::cerr;
using std::endl;
using std::string;

constexpr size_t BUF_SIZE = 256;

// Read the entire contents of an istream into a string. Reads BUF_SIZE bytes at a time.
static inline string read_istream(std::istream& stream) {
    std::string retval;
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
