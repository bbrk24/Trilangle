#pragma once

#include "int24.hh"
#include <string>

struct flags {
	bool debug;
	bool show_stack;
	bool warnings;

	constexpr flags() noexcept : debug(false), show_stack(false), warnings(false) { }
};

// Read input file or STDIN, and return its contents. Parse other flags as appropriate.
std::string parse_args(int argc, char** argv, flags& f);

// Gets a single unicode character from STDIN. Returns -1 for EOF.
int24_t getunichar() noexcept;
