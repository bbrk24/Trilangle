#pragma once

#include "int24.hh"
#include <string>

struct flags {
	unsigned char debug : 1;
};

// Read input file or STDIN, and return its contents. Parse other flags as appropriate.
std::string parse_args(int argc, char** argv, flags& f) noexcept;

// Gets a single unicode character from STDIN. Returns -1 for EOF.
int24_t getunichar() noexcept;
