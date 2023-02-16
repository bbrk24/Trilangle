#pragma once

#include "int24.hh"
#include <string>

struct flags {
	bool debug : 1;
	bool show_stack : 1;
	bool warnings : 1;
	bool pipekill : 1;

	constexpr flags() noexcept : debug(false), show_stack(false), warnings(false), pipekill(false) { }
};

// Read input file or STDIN, and return its contents. Parse other flags as appropriate.
std::string parse_args(int argc, char** argv, flags& f);

// Gets a single unicode character from STDIN. Returns -1 for EOF.
int24_t getunichar() noexcept;
