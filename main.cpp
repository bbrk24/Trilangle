#include "interpreter.hh"

int main(int argc, char** argv) {
    flags f{ 0, 0 };
    program p(parse_args(argc, argv, f));
    interpreter i(p, f);

    i.run();
}
