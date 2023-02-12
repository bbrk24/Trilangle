#include "input.hh"
#include "interpreter.hh"
#include <iostream>

int main(int argc, char** argv) {
    program p(readfile(argc, argv));
    interpreter i(p);

    i.run();
}
