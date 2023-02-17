#define _CRT_SECURE_NO_WARNINGS 1

#include "interpreter.hh"
#include "opcode.hh"
#include "string_processing.hh"
#include <iostream>
#include <cstdlib>
#include <cinttypes>
#include <random>
#include <type_traits>
#include <functional>

#define EMPTY_PROTECT(name) \
    if (m_stack.empty() && m_flags.warnings) UNLIKELY { \
        cerr << "Warning: Attempt to " name " empty stack.\n"; \
    } else

#define SIZE_CHECK(name, count) \
    if (m_flags.warnings && m_stack.size() < (count)) \
       UNLIKELY cerr << "Warning: Attempt to " name " stack with fewer than " << (count) << " elements.\n"

#ifdef SCNi24
using input_type = int24_t;
constexpr const char* FORMAT_STRING = "%" SCNi24;
#else
using input_type = int32_t;
constexpr const char* FORMAT_STRING = "%" SCNi32;
#endif

using rand_type = std::conditional_t<std::is_integral<int24_t>::value, int24_t, int32_t>;

using std::cout;
using std::cerr;

// Advance the IP one step, accounting for wrap-around.
void interpreter::advance() noexcept {
    // I don't remember how this works, it just does.
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
    // Create the random number generator.

    std::default_random_engine reng(std::invoke(std::random_device()));
    std::uniform_int_distribution<rand_type> rdist(-0x800000, 0x7fffff);

    // Begin the execution loop.
    while (true) {
        // The operation currently being executed
        int24_t op = m_program.at(m_coords.first, m_coords.second);

        // Print the requisite information in debug mode.
        if (m_flags.debug) {
            if (m_flags.show_stack) {
                cout << "Stack: [";

                for (size_t i = 0; i < m_stack.size(); ++i) {
                    if (i != 0) {
                        cout << ", ";
                    }
                    cout << m_stack[i];
                }

                cout << "]\n";
            }

            cout << "Coords: (" << m_coords.first << ", " << m_coords.second << ")\nInstruction: " << std::flush;
            putwchar(static_cast<wchar_t>(op));
            putchar('\n');

            DISCARD getchar();
        }

        // ...yeah
        switch (op) {
            case NOP:
                break;
            case ADD: {
                SIZE_CHECK("add from", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() += top;
                break;
            }
            case SUB: {
                SIZE_CHECK("subtract from", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() -= top;
                break;
            }
            case MUL: {
                SIZE_CHECK("multiply from", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() *= top;
                break;
            }
            case DIV: {
                SIZE_CHECK("divide from", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() /= top;
                break;
            }
            case MOD: {
                SIZE_CHECK("divide from", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() %= top;
                break;
            }
            case MIR_EW:
                switch (m_direction) {
                    case direction::southwest:
                        m_direction = direction::northwest;
                        break;
                    case direction::northwest:
                        m_direction = direction::southwest;
                        break;
                    case direction::northeast:
                        m_direction = direction::southeast;
                        break;
                    case direction::southeast:
                        m_direction = direction::northeast;
                        break;
                    case direction::east: FALLTHROUGH
                    case direction::west:
                        break;
                }
                break;
            case MIR_NS:
                switch (m_direction) {
                    case direction::southwest:
                        m_direction = direction::southeast;
                        break;
                    case direction::northwest:
                        m_direction = direction::northeast;
                        break;
                    case direction::northeast:
                        m_direction = direction::northwest;
                        break;
                    case direction::southeast:
                        m_direction = direction::southwest;
                        break;
                    case direction::west:
                        m_direction = direction::east;
                        break;
                    case direction::east:
                        m_direction = direction::west;
                        break;
                }
                break;
            case MIR_NESW:
                switch (m_direction) {
                    case direction::west:
                        m_direction = direction::southeast;
                        break;
                    case direction::southeast:
                        m_direction = direction::west;
                        break;
                    case direction::east:
                        m_direction = direction::northwest;
                        break;
                    case direction::northwest:
                        m_direction = direction::east;
                        break;
                    case direction::northeast: FALLTHROUGH
                    case direction::southwest:
                        break;
                }
                break;
            case MIR_NWSE:
                switch (m_direction) {
                    case direction::west:
                        m_direction = direction::northeast;
                        break;
                    case direction::northeast:
                        m_direction = direction::west;
                        break;
                    case direction::east:
                        m_direction = direction::southwest;
                        break;
                    case direction::southwest:
                        m_direction = direction::east;
                        break;
                    case direction::northwest: FALLTHROUGH
                    case direction::southeast:
                        break;
                }
                break;
            case BNG_E:
                switch (m_direction) {
                    case direction::west:
                        EMPTY_PROTECT("branch on");
                        if (m_stack.back() < INT24_C(0)) {
                            m_direction = direction::southwest;
                        } else {
                            m_direction = direction::northwest;
                        }
                        break;
                    case direction::northeast: FALLTHROUGH
                    case direction::southeast:
                        m_direction = direction::east;
                        break;
                    case direction::east:
                        m_direction = direction::west;
                        break;
                    case direction::southwest:
                        m_direction = direction::northeast;
                        break;
                    case direction::northwest:
                        m_direction = direction::southeast;
                        break;
                }
                break;
            case BNG_W:
                switch (m_direction) {
                    case direction::east:
                        EMPTY_PROTECT("branch on");
                        if (m_stack.back() < INT24_C(0)) {
                            m_direction = direction::northeast;
                        } else {
                            m_direction = direction::southeast;
                        }
                        break;
                    case direction::northwest: FALLTHROUGH
                    case direction::southwest:
                        m_direction = direction::west;
                        break;
                    case direction::west:
                        m_direction = direction::east;
                        break;
                    case direction::southeast:
                        m_direction = direction::northwest;
                        break;
                    case direction::northeast:
                        m_direction = direction::southwest;
                        break;
                }
                break;
            case BNG_NE:
                switch (m_direction) {
                    case direction::southwest:
                        EMPTY_PROTECT("branch on");
                        if (m_stack.back() < INT24_C(0)) {
                            m_direction = direction::southeast;
                        } else {
                            m_direction = direction::west;
                        }
                        break;
                    case direction::northwest: FALLTHROUGH
                    case direction::east:
                        m_direction = direction::northeast;
                        break;
                    case direction::northeast:
                        m_direction = direction::southwest;
                        break;
                    case direction::west:
                        m_direction = direction::east;
                        break;
                    case direction::southeast:
                        m_direction = direction::northwest;
                        break;
                }
                break;
            case BNG_SW:
                switch (m_direction) {
                    case direction::northeast:
                        EMPTY_PROTECT("branch on");
                        if (m_stack.back() < INT24_C(0)) {
                            m_direction = direction::northwest;
                        } else {
                            m_direction = direction::east;
                        }
                        break;
                    case direction::southeast: FALLTHROUGH
                    case direction::west:
                        m_direction = direction::southwest;
                        break;
                    case direction::northwest:
                        m_direction = direction::southeast;
                        break;
                    case direction::east:
                        m_direction = direction::west;
                        break;
                    case direction::southwest:
                        m_direction = direction::northeast;
                        break;
                }
                break;
            case BNG_NW:
                switch (m_direction) {
                    case direction::southeast:
                        EMPTY_PROTECT("branch on");
                        if (m_stack.back() < INT24_C(0)) {
                            m_direction = direction::east;
                        } else {
                            m_direction = direction::southwest;
                        }
                        break;
                    case direction::northeast: FALLTHROUGH
                    case direction::west:
                        m_direction = direction::northwest;
                        break;
                    case direction::northwest:
                        m_direction = direction::southeast;
                        break;
                    case direction::east:
                        m_direction = direction::west;
                        break;
                    case direction::southwest:
                        m_direction = direction::northeast;
                        break;
                }
                break;
            case BNG_SE:
                switch (m_direction) {
                    case direction::northwest:
                        EMPTY_PROTECT("branch on");
                        if (m_stack.back() < INT24_C(0)) {
                            m_direction = direction::west;
                        } else {
                            m_direction = direction::northeast;
                        }
                        break;
                    case direction::southwest: FALLTHROUGH
                    case direction::east:
                        m_direction = direction::southeast;
                        break;
                    case direction::northeast:
                        m_direction = direction::southwest;
                        break;
                    case direction::west:
                        m_direction = direction::east;
                        break;
                    case direction::southeast:
                        m_direction = direction::northwest;
                        break;
                }
                break;
            case PSC: {
                advance();
                int24_t next = m_program.at(m_coords.first, m_coords.second);
                m_stack.push_back(next);
                break;
            }
            case PSI: {
                advance();
                int24_t next = m_program.at(m_coords.first, m_coords.second);

                if (m_flags.warnings) {
                    if (next < (int24_t)'0' || next > (int24_t)'9') UNLIKELY {
                        cerr << "Warning: Pushing non-decimal number with " << static_cast<char>(opcode::PSI) <<
                            " is implementation-defined behavior.\n";
                    }
                }

                m_stack.emplace_back(next - '0');
                break;
            }
            case POP:
                EMPTY_PROTECT("pop from")
                    m_stack.pop_back();
                break;
            case EXT:
                return;
            case INC:
                EMPTY_PROTECT("increment")
                    m_stack.back() += INT24_C(1);
                break;
            case DEC:
                EMPTY_PROTECT("decrement")
                    m_stack.back() -= INT24_C(1);
                break;
            case AND: {
                SIZE_CHECK("bitwise and", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() &= top;
                break;
            }
            case IOR: {
                SIZE_CHECK("bitwise or", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() |= top;
                break;
            }
            case XOR: {
                SIZE_CHECK("bitwise xor", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() ^= top;
                break;
            }
            case NOT:
                EMPTY_PROTECT("complement")
                    m_stack.back() = ~m_stack.back();
                break;
            case GTC:
                m_stack.push_back(getunichar());
                break;
            case PTC:
                EMPTY_PROTECT("print from")
                    putwchar(static_cast<wchar_t>(m_stack.back()));

                if (m_flags.pipekill && ferror(stdout)) {
                    return;
                }

                break;
            case GTI: {
                input_type i;

                while (!scanf(FORMAT_STRING, &i)) {
                    if (feof(stdin)) {
                        i = -1;
                        break;
                    }

                    DISCARD getchar();
                }

                m_stack.emplace_back(i);
                break;
            }
            case PTI:
                EMPTY_PROTECT("print from")
                    printf("%" PRId32 "\n", static_cast<int32_t>(m_stack.back()));

                if (m_flags.pipekill && ferror(stdout)) {
                    return;
                }

                break;
            case SKP:
                advance();
                break;
            case IDX: {
                int24_t top = m_stack.back();
                m_stack.pop_back();

                if (m_flags.warnings && top < INT24_C(0)) UNLIKELY {
                    cerr << "Warning: Attempt to use negative index.\n";
                } else SIZE_CHECK("index", static_cast<unsigned int>(top) + 1);

                size_t i = m_stack.size() - top - 1;
                m_stack.push_back(m_stack[i]);
                break;
            }
            case DUP:
                EMPTY_PROTECT("duplicate")
                    m_stack.push_back(m_stack.back());
                break;
            case RND:
                m_stack.emplace_back(rdist(reng));
                break;
            case EXP:
                EMPTY_PROTECT("exponentiate")
                    m_stack.back() = INT24_C(1) << m_stack.back();
                break;
            case SWP: {
                SIZE_CHECK("swap in", 2);
                size_t i = m_stack.size() - 2;
                std::swap(m_stack[i], m_stack[i + 1]);
                break;
            }
            UNLIKELY case INVALID_CHAR:
                fprintf(
                    stderr,
                    "Unicode replacement character (U+%0.4X) detected in source. Please check encoding.\n",
                    static_cast<unsigned int>(op)
                );
                exit(1);
            default:
                cerr << "Unrecognized opcode '" << std::flush;
                putwchar(static_cast<wchar_t>(op));
                cerr << "' (at (" << m_coords.first << ", " << m_coords.second << "))" << std::endl;
                exit(1);
        }
        
        advance();
    }
}
