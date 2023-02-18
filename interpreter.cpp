#define _CRT_SECURE_NO_WARNINGS 1

#include "interpreter.hh"
#include <iostream>
#include <cstdlib>
#include <cinttypes>
#include <random>
#include <type_traits>

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

void interpreter::run() {
    // Create the random number generator.

    std::default_random_engine reng(std::move(std::random_device())());
    std::uniform_int_distribution<rand_type> rdist(-0x800000, 0x7fffff);

    // Begin the execution loop.
    while (true) {
        // The operation currently being executed
        int24_t op = m_program.at(m_ip.coords.first, m_ip.coords.second);

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

            cout << "Coords: (" << m_ip.coords.first << ", " << m_ip.coords.second << ")\nInstruction: " << std::flush;
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
            case MIR_EW: FALLTHROUGH
            case MIR_NESW: FALLTHROUGH
            case MIR_NS: FALLTHROUGH
            case MIR_NWSE:
                program_walker::reflect(m_ip.dir, op);
                break;
            case BNG_E: FALLTHROUGH
            case BNG_NE: FALLTHROUGH
            case BNG_NW: FALLTHROUGH
            case BNG_SE: FALLTHROUGH
            case BNG_SW: FALLTHROUGH
            case BNG_W:
                program_walker::branch(m_ip.dir, op, [&]() {
                    EMPTY_PROTECT("branch on");
                    return m_stack.back() < INT24_C(0);
                });
                break;
            case PSC: {
                advance();
                int24_t next = m_program.at(m_ip.coords.first, m_ip.coords.second);
                m_stack.push_back(next);
                break;
            }
            case PSI: {
                advance();
                int24_t next = m_program.at(m_ip.coords.first, m_ip.coords.second);

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
                cerr << "' (at (" << m_ip.coords.first << ", " << m_ip.coords.second << "))" << std::endl;
                exit(1);
        }
        
        advance();
    }
}
