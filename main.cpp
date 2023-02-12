#include "input.hh"
#include "program.hh"
#include <iostream>

int main(int argc, char** argv) {
    std::string input = readfile(argc, argv);
    std::cout << "Input:\n" << input << std::endl;

    program p(input);
    std::cout << "Triangle side length: " << p.side_length() << std::endl;

    for (size_t i = 0; i < p.side_length(); ++i) {
        for (size_t j = 0; j <= 1 + p.side_length() / 2 - i; ++j) {
            std::cout << ' ';
        }

        for (size_t j = 0; j <= i; ++j) {
            std::cout << (char)p.at(i, j) << ' ';
        }
        std::cout << '\n';
    }
}
