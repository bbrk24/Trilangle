#include "input.hh"
#include <iostream>

int main(int argc, char** argv) {
    std::string input = readfile(argc, argv);
    std::cout << "Input:\n" << input << std::endl;
}
