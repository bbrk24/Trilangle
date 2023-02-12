#include "interpreter.hh"
#include <iostream>

interpreter::interpreter(const program& p, flags f) noexcept :
	m_program(p),
	m_coords{ 0, 0 },
	m_stack(),
	m_direction(direction::southwest),
	m_flags(f) { }

void interpreter::advance() noexcept {
	switch (m_direction) {
		case direction::southwest:
			if (++m_coords.first == m_program.side_length()) {
				m_coords.second = (m_coords.second + 1) % m_program.side_length();
				m_coords.first = m_coords.second;
			}
			break;
		case direction::west:
			if (m_coords.second == 0) {
				if (++m_coords.first == m_program.side_length()) {
					m_coords.first = 0;
				} else {
					m_coords.second = m_coords.first;
				}
			} else {
				--m_coords.second;
			}
			break;
		case direction::northwest:
			if (m_coords.second == 0) {
				if (m_coords.first == m_program.side_length() - 1) {
					m_coords.second = m_program.side_length() - 1;
				} else {
					m_coords.second = m_program.side_length() - m_coords.first - 2;
					m_coords.first = m_program.side_length() - 1;
				}
			} else {
				--m_coords.first;
				--m_coords.second;
			}
			break;
		case direction::northeast:
			if (m_coords.first == 0 || m_coords.second >= m_coords.first) {
				m_coords.first = m_program.side_length() - 1;
				if (m_coords.second == 0) {
					m_coords.second = m_program.side_length() - 1;
				} else {
					--m_coords.second;
				}
			} else {
				--m_coords.first;
			}
			break;
		case direction::east:
			if (++m_coords.second > m_coords.first) {
				if (m_coords.first == 0) {
					m_coords.first = m_program.side_length() - 1;
				} else {
					--m_coords.first;
				}
				m_coords.second = 0;
			}
			break;
		case direction::southeast:
			++m_coords.second;
			if (++m_coords.first == m_program.side_length()) {
				if (m_coords.second < m_program.side_length()) {
					m_coords.first = m_program.side_length() - m_coords.second - 1;
				} else {
					m_coords.first = m_program.side_length() - 1;
				}
				m_coords.second = 0;
			}
			break;
	}
}

void interpreter::run() {
	for (size_t i = 0; i <= m_program.side_length() * (m_program.side_length() + 1) / 2; ++i) {
		int24_t op = m_program.at(m_coords.first, m_coords.second);
		if (m_flags.debug) {
			std::cout << "Coords: (" << m_coords.first << ", " << m_coords.second << ")\nInstruction: ";
		}
		std::wcout << (wchar_t)op << std::endl;
		advance();
	}
}
