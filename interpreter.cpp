#define _CRT_SECURE_NO_WARNINGS 1

#include "interpreter.hh"
#include "opcodes.hh"
#include "string_processing.hh"
#include <iostream>
#include <cstdlib>
#include <cinttypes>

using std::cerr;
using std::endl;

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
            std::wcout << (wchar_t)op << endl;
        }

        switch (op) {
            case NOP:
                break;
            case ADD: {
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() += top;
                break;
            }
            case SUB: {
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() -= top;
                break;
            }
            case MUL: {
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() *= top;
                break;
            }
            case DIV: {
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() /= top;
                break;
            }
            case MOD: {
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() %= top;
                break;
            }
            // BSN and MIR
            case PSH: {
                advance();
                int24_t next = m_program.at(m_coords.first, m_coords.second);
                m_stack.push_back(next);
                break;
            }
            case POP:
                m_stack.pop_back();
                break;
            case EXT:
                return;
            case INC:
                m_stack.back() += INT24_C(1);
                break;
            case DEC:
                m_stack.back() -= INT24_C(1);
                break;
            case AND: {
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() &= top;
                break;
            }
            // IOR and XOR
            case NOT:
                m_stack.back() = ~m_stack.back();
                break;
            case GTC:
                m_stack.push_back(parse_unichar(getchar));
                break;
            case PTC:
                std::wcout << (wchar_t)m_stack.back();
                break;
            case GTI: {
                int32_t i;
                
                while (!scanf("%" SCNi32, &i)) {
                    if (feof(stdin)) {
                        i = -1;
                        break;
                    }

                    [[maybe_unused]] int _ = scanf("%*c");
                }

                m_stack.push_back(int24_t{ i });
                break;
            }
            case PTI:
                std::cout << m_stack.back() << endl;
                break;
            case SKP:
                advance();
                break;
            case LEA: {
                int24_t top = m_stack.back();
                m_stack.pop_back();
                size_t i = m_stack.size() - top - 1;
                m_stack.push_back(m_stack[i]);
                break;
            }
            case 0xfffd:
                cerr << "Unicode replacement character (U+FFFD) detected in source. Please check encoding." << endl;
                exit(1);
            default:
                cerr << "Unrecognized opcode '";
                std::wcerr << (wchar_t)op;
                cerr << "' (at (" << m_coords.first << ", " << m_coords.second << "))" << endl;
                exit(1);
        }
        
        advance();
    }
}
