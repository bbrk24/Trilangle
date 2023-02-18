#include "interpreter.hh"
#include "disassembler.hh"
#include <iostream>

int main(int argc, char** argv) {
    flags f;
    program p(parse_args(argc, argv, f));

    if (f.disassemble) {
        disassembler(p, f).write_state(std::wcout);
    } else {
        interpreter i(p, f);
        i.run();
    }
}
