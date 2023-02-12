#pragma once

#include "int24.hh"
#include <string>

// If one argument is passed, use it as a filename and read its contents.
// If no arguments are passed, read from STDIN until EOF.
// If more than one argument is passed, terminates the program with exit code 1.
std::string readfile(int argc, char** argv);

// Gets a single unicode character from STDIN. Returns -1 for EOF.
int24_t getunichar() noexcept;
