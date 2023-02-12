#pragma once

#include "program.hh"
#include "input.hh"
#include <utility>

enum class direction : char {
	southwest, west, northwest, northeast, east, southeast
};

class interpreter {
public:
	interpreter(const program& p, flags f) noexcept;

	void run();
private:
	void advance() noexcept;

	const program& m_program;
	std::pair<size_t, size_t> m_coords;
	std::vector<int24_t> m_stack;
	direction m_direction;
	flags m_flags;
};
