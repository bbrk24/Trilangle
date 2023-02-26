#define _CRT_SECURE_NO_WARNINGS 1

#include "interpreter.hh"
#include <cstdlib>
#include <cinttypes>
#include <random>
#include <type_traits>

// When targeting the web, the page can't re-render until the C++ code finishes.
// emscripten_sleep is asynchronous (so it allows a re-render), but it looks synchronous from the C side.
// Insert a call to emscripten_sleep(0) whenever the page needs a chance to redraw, such as after a print statement.
#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define emscripten_sleep(x) ((void)0)
#endif

#define EMPTY_PROTECT(name, sleep) \
    if (m_stack.empty() && m_flags.warnings) UNLIKELY { \
        cerr << "Warning: Attempt to " name " empty stack.\n"; \
        if (sleep) emscripten_sleep(0); \
    } else

#define SIZE_CHECK(name, count) \
    if (m_flags.warnings && m_stack.size() < (count)) \
       UNLIKELY cerr << "Warning: Attempt to " name " stack with fewer than " << (count) << " elements.\n"

using std::cout;
using std::cerr;

constexpr int24_t INT24_MIN{ -0x800000 };
constexpr int24_t INT24_MAX{ 0x7fffff };

static bool should_run;

void interpreter::stop_all() noexcept {
    should_run = false;
}

void interpreter::run() {
    should_run = true;

    // Create the random number generator.

    std::default_random_engine reng(std::move(std::random_device())());
    static std::uniform_int_distribution<int32_t> rdist(INT24_MIN, INT24_MAX);

    // Begin the execution loop.
    while (should_run) {
        // The operation currently being executed
        int24_t op = m_program.at(m_ip.coords.first, m_ip.coords.second);

        // Print the requisite information in debug mode.
        if (m_flags.debug) {
            // TODO: Figure out debugging on the web
#ifndef __EMSCRIPTEN__
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

            cout << "Coords: (" << m_ip.coords.first << ", " << m_ip.coords.second << ")\nInstruction: ";
            printunichar(op);
            cout << std::endl;

            DISCARD getchar();
#endif
        }

        // ...yeah
        switch (op) {
            case NOP:
                break;
            case ADD: {
                SIZE_CHECK("add from", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();

                int32_t temp = m_stack.back() + top;

                if (m_flags.warnings && (temp < INT24_MIN || temp > INT24_MAX)) UNLIKELY {
                    cerr << "Warning: Overflow on addition/subtraction is undefined behavior.\n";
                }

                m_stack.back() = static_cast<int24_t>(temp);

                break;
            }
            case SUB: {
                SIZE_CHECK("subtract from", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();

                int32_t temp = m_stack.back() - top;

                if (m_flags.warnings && (temp < INT24_MIN || temp > INT24_MAX)) UNLIKELY {
                    cerr << "Warning: Overflow on addition/subtraction is undefined behavior.\n";
                }

                m_stack.back() = static_cast<int24_t>(temp);

                break;
            }
            case MUL: {
                SIZE_CHECK("multiply from", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();

                int64_t temp = static_cast<int64_t>(m_stack.back()) * static_cast<int64_t>(top);

                if (m_flags.warnings && (temp < INT24_MIN || temp > INT24_MAX)) UNLIKELY {
                    cerr << "Warning: Overflow on multiplication is undefined behavior.\n";
                }

                m_stack.back() = static_cast<int24_t>(temp);

                break;
            }
            case DIV: {
                if (m_flags.warnings) {
                    if (m_stack.size() < 2) UNLIKELY {
                        cerr << "Warning: Attempt to divide from stack with fewer than 2 elements.\n";
                    }
                    if (!m_stack.empty() && m_stack.back() == INT24_C(0)) UNLIKELY {
                        cerr << "Warning: Attempted division by zero.\n";
                    }
                }
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() /= top;
                break;
            }
            case MOD: {
                if (m_flags.warnings) {
                    if (m_stack.size() < 2) UNLIKELY {
                        cerr << "Warning: Attempt to divide from stack with fewer than 2 elements.\n";
                    }
                    if (!m_stack.empty() && m_stack.back() == INT24_C(0)) UNLIKELY {
                        cerr << "Warning: Attempted division by zero.\n";
                    }
                }
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
                program_walker::branch(m_ip.dir, op, [&]() NOEXCEPT_T {
                    EMPTY_PROTECT("branch on", false) {}
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
                        cerr << "Warning: Pushing non-ASCII-decimal number with " << static_cast<char>(opcode::PSI)
                            << " is implementation-defined behavior.\n";
                    }
                }

                m_stack.emplace_back(next - '0');
                break;
            }
            case POP:
                EMPTY_PROTECT("pop from", true) {
                    m_stack.pop_back();
                }
                break;
            case EXT:
                // Emscripten doesn't flush after every putchar call, so ensure we flush at all
                cout << std::flush;
                return;
            case INC:
                if (m_flags.warnings) {
                    if (m_stack.empty()) UNLIKELY {
                        cerr << "Warning: Attempt to increment empty stack.\n";
                        emscripten_sleep(0);
                        break;
                    }
                    if (m_stack.back() == INT24_MAX) UNLIKELY {
                        cerr << "Warning: Overflow on addition/subtraction is undefined behavior.\n";
                    }
                }

                ++m_stack.back();

                break;
            case DEC:
                if (m_flags.warnings) {
                    if (m_stack.empty()) UNLIKELY {
                        cerr << "Warning: Attempt to decrement empty stack.\n";
                        emscripten_sleep(0);
                        break;
                    }
                    if (m_stack.back() == INT24_MAX) UNLIKELY {
                        cerr << "Warning: Overflow on addition/subtraction is undefined behavior.\n";
                    }
                }

                --m_stack.back();

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
                EMPTY_PROTECT("complement", true) {
                    m_stack.back() = ~m_stack.back();
                }
                break;
            case GTC:
                m_stack.push_back(getunichar());
                break;
            case PTC:
                EMPTY_PROTECT("print from", false) {
                    printunichar(m_stack.back());
                }

                if (m_flags.pipekill && ferror(stdout)) {
                    return;
                }

                emscripten_sleep(0);

                break;
            case GTI: {
                int32_t i = -1;

                while (!(feof(stdin) || scanf("%" SCNi32, &i))) {
                    DISCARD getchar();
                }

                m_stack.emplace_back(i);
                break;
            }
            case PTI:
                EMPTY_PROTECT("print from", false) {
                    printf("%" PRId32 "\n", static_cast<int32_t>(m_stack.back()));
                }

                if (m_flags.pipekill && ferror(stdout)) {
                    return;
                }

                emscripten_sleep(0);

                break;
            case SKP:
                advance();
                break;
            case IDX: {
                if (m_flags.warnings && m_stack.empty()) UNLIKELY {
                    cerr << "Warning: Attempt to read index from empty stack.\n";
                    emscripten_sleep(0);
                    break;
                }

                int24_t top = m_stack.back();
                m_stack.pop_back();

                if (m_flags.warnings) {
                    if (top < INT24_C(0) || m_stack.size() < top + 1) UNLIKELY {
                        cerr << "Warning: Attempt to index out of stack bounds (size = " << m_stack.size()
                            << ", index = " << top << ")\n";
                    }
                }

                size_t i = m_stack.size() - top - 1;
                m_stack.push_back(m_stack[i]);

                break;
            }
            case DUP:
                EMPTY_PROTECT("duplicate", true) {
                    m_stack.push_back(m_stack.back());
                }
                break;
            case RND:
                m_stack.emplace_back(rdist(reng));
                break;
            case EXP:
                EMPTY_PROTECT("exponentiate", true) {
                    m_stack.back() = INT24_C(1) << m_stack.back();
                }
                break;
            case SWP: {
                SIZE_CHECK("swap in", 2);
                size_t i = m_stack.size() - 2;
                std::swap(m_stack[i], m_stack[i + 1]);
                break;
            }
            case INVALID_CHAR:
                fprintf(
                    stderr,
                    "Unicode replacement character (U+%0.4" PRIX32 ") detected in source. Please check encoding.\n",
                    static_cast<uint32_t>(op)
                );
                exit(1);
            default:
                cerr << "Unrecognized opcode '";
                printunichar(op, cerr);
                cerr << "' (at (" << m_ip.coords.first << ", " << m_ip.coords.second << "))" << std::endl;
                exit(1);
        }

        advance();
    }
}
