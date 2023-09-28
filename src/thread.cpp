#define _CRT_SECURE_NO_WARNINGS 1

#include "thread.hh"
#include <chrono>
#include <cinttypes>
#include <cstdlib>
#include <random>
#include "output.hh"
#include "time.hh"

#ifndef TRILANGLE_CLOCK
#define TRILANGLE_CLOCK std::chrono::system_clock
#endif

#define EMPTY_PROTECT(name) \
    if (m_stack.empty() && m_flags.warnings) UNLIKELY { \
        cerr << "Warning: Attempt to " name " empty stack.\n"; \
    } else

#define SIZE_CHECK(name, count) \
    if (m_flags.warnings && m_stack.size() < (count)) UNLIKELY \
    cerr << "Warning: Attempt to " name " stack with fewer than " << (count) << " elements.\n"

using std::cerr;
using std::cout;
using std::flush;
using std::pair;
using trilangle_clock = TRILANGLE_CLOCK;

using status = thread::status;

unsigned long thread::thread_count;

#ifndef __EMSCRIPTEN__
extern "C" void send_debug_info(
    unsigned long thread_number,
    const int24_t* stack,
    size_t stack_depth,
    size_t y,
    size_t x,
    int32_t instruction
) {
    cout << "Thread " << thread_number << '\n';
    if (stack != nullptr) {
        cout << "Stack: [";

        for (size_t i = 0; i < stack_depth; ++i) {
            if (i != 0) {
                cout << ", ";
            }
            cout << stack[i];
        }

        cout << "]\n";
    }

    cout << "Coords: (" << x << ", " << y << ")\nInstruction: ";
    print_unichar(static_cast<int24_t>(instruction));
    cout << std::endl;

    DISCARD getchar();
}
#endif

// Emscripten doesn't flush after every putchar call, so ensure we flush at all
[[noreturn]] static inline void flush_and_exit(int code) {
    cout << flush;
    cerr << flush;
#ifdef __EMSCRIPTEN__
    // https://github.com/bbrk24/Trilangle/issues/4
    while (!feof(stdin)) {
        DISCARD getchar();
    }
#endif
    exit(code);
}

void thread::tick() {
    static std::default_random_engine engine((std::random_device())());
    static std::uniform_int_distribution<int32_t> distr(INT24_MIN, INT24_MAX);

    // The operation currently being executed
    int24_t op = m_program->at(m_ip.coords.first, m_ip.coords.second);

    // The web interface needs every thread to send debug info. The CLI expects only active threads to do so.
#ifdef __EMSCRIPTEN__
    if (m_flags.debug) {
        send_debug_info(
            m_number,
            m_flags.show_stack ? m_stack.data() : nullptr,
            m_stack.size(),
            m_ip.coords.first,
            m_ip.coords.second,
            op
        );
    }
#endif

    switch (m_status) {
        case status::idle:
            m_status = status::active;
            return;
        case status::waiting:
            return;
        case status::active:
            break;
        case status::terminated:
            unreachable("tick() should not be called on terminated thread");
        case status::splitting:
            unreachable("thread with m_status = splitting should be split");
    }

#ifndef __EMSCRIPTEN__
    // Print the requisite information in debug mode.
    if (m_flags.debug) {
        send_debug_info(
            m_number,
            m_flags.show_stack ? m_stack.data() : nullptr,
            m_stack.size(),
            m_ip.coords.first,
            m_ip.coords.second,
            op
        );
    }
#endif

    // ...yeah
    switch (op) {
        case NOP:
            break;
        case ADD: {
            SIZE_CHECK("add from", 2);
            int24_t top = m_stack.back();
            m_stack.pop_back();

            pair<bool, int24_t> result = m_stack.back().add_with_overflow(top);

            if (m_flags.warnings && result.first) UNLIKELY {
                cerr << "Warning: Overflow on addition/subtraction is undefined behavior.\n";
            }

            m_stack.back() = result.second;

            break;
        }
        case SUB: {
            SIZE_CHECK("subtract from", 2);
            int24_t top = m_stack.back();
            m_stack.pop_back();

            pair<bool, int24_t> result = m_stack.back().subtract_with_overflow(top);

            if (m_flags.warnings && result.first) UNLIKELY {
                cerr << "Warning: Overflow on addition/subtraction is undefined behavior.\n";
            }

            m_stack.back() = result.second;

            break;
        }
        case MUL: {
            SIZE_CHECK("multiply from", 2);
            int24_t top = m_stack.back();
            m_stack.pop_back();

            pair<bool, int24_t> result = m_stack.back().multiply_with_overflow(top);

            if (result.first) UNLIKELY {
                cerr << "Warning: Overflow on multiplication is undefined behavior.\n";
            }

            m_stack.back() = result.second;

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

            if (m_flags.warnings && m_stack.back() == INT24_MIN && top == INT24_C(-1)) UNLIKELY {
                cerr << "Warning: Overflow on division is undefined behavior.\n";
            }

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
        case MIR_EW:
        case MIR_NESW:
        case MIR_NS:
        case MIR_NWSE:
            program_walker::reflect(m_ip.dir, op);
            break;
        case BNG_E:
        case BNG_NE:
        case BNG_NW:
        case BNG_SE:
        case BNG_SW:
        case BNG_W:
            program_walker::branch(m_ip.dir, op, [&]() NOEXCEPT_T {
                EMPTY_PROTECT("branch on") {}
                return m_stack.back() < INT24_C(0);
            });
            break;
        case PSC: {
            advance();
            m_status = status::idle;
            int24_t next = m_program->at(m_ip.coords.first, m_ip.coords.second);
            m_stack.push_back(next);
            break;
        }
        case PSI: {
            advance();
            m_status = status::idle;
            int24_t next = m_program->at(m_ip.coords.first, m_ip.coords.second);

            if (m_flags.warnings) {
                if (next < (int24_t)'0' || next > (int24_t)'9') UNLIKELY {
                    cerr << "Warning: Pushing non-ASCII-decimal number with " << static_cast<char>(opcode::PSI)
                         << " is implementation-defined behavior.\n";
                }
            }

            m_stack.push_back(next - (int24_t)'0');
            break;
        }
        case POP:
            EMPTY_PROTECT("pop from") {
                m_stack.pop_back();
            }
            break;
        case EXT:
            flush_and_exit(EXIT_SUCCESS);
        case INC:
            EMPTY_PROTECT("increment") {
                ++m_stack.back();
            }
            break;
        case DEC:
            EMPTY_PROTECT("decrement") {
                --m_stack.back();
            }
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
            EMPTY_PROTECT("complement") {
                m_stack.back() = ~m_stack.back();
            }
            break;
        case GTC:
            if (m_flags.assume_ascii) {
                int character = getchar();
                if (character == EOF) {
                    m_stack.push_back(INT24_C(-1));
                } else {
                    if (m_flags.warnings && (character & 0x7f) != character) UNLIKELY {
                        cerr << "Non-ASCII byte read.\n";
                    }
                    m_stack.emplace_back(character);
                }
            } else {
                m_stack.push_back(get_unichar());
            }
            break;
        case PTC:
            EMPTY_PROTECT("print from") {
                bool should_print = true;

                if (m_stack.back() < INT24_C(0)) UNLIKELY {
                    if (m_flags.warnings) {
                        cerr << "Warning: Attempt to print character with negative value.\n";
                        should_print = false;
                    }
                    if (m_flags.pipekill) {
                        flush_and_exit(EXIT_FAILURE);
                    }
                }

                if (should_print) {
                    if (m_flags.assume_ascii) {
                        int24_t back = m_stack.back();
                        if (m_flags.warnings && (back & 0x7f) != back) UNLIKELY {
                            cerr << "Warning: Printing non-ASCII value.\n";
                        }
                        putchar(back);
                    } else {
                        print_unichar(m_stack.back());
                    }

#ifdef __EMSCRIPTEN__
                    if (m_flags.debug) {
                        cout << flush;
                    }
#endif
                }
            }

            if (m_flags.pipekill && ferror(stdout)) {
                // No need to flush stdout, it's already closed
                cerr << flush;
                exit(EXIT_SUCCESS);
            }

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
            EMPTY_PROTECT("print from") {
                cout << m_stack.back() << '\n';
            }

            if (m_flags.pipekill && ferror(stdout)) {
                cerr << flush;
                exit(EXIT_SUCCESS);
            }
            break;
        case SKP:
            advance();
            m_status = status::idle;
            break;
        case IDX: {
            if (m_flags.warnings && m_stack.empty()) UNLIKELY {
                cerr << "Warning: Attempt to read index from empty stack.\n";
                break;
            }

            int24_t top = m_stack.back();
            m_stack.pop_back();

            if (m_flags.warnings && (top < INT24_C(0) || m_stack.size() < static_cast<size_t>(top) + 1)) UNLIKELY {
                cerr << "Warning: Attempt to index out of stack bounds (size = " << m_stack.size()
                     << ", index = " << top << ")\n";
            }

            size_t i = m_stack.size() - static_cast<size_t>(top) - 1;
            m_stack.push_back(m_stack[i]);

            break;
        }
        case DUP:
            EMPTY_PROTECT("duplicate") {
                m_stack.push_back(m_stack.back());
            }
            break;
        case RND:
            m_stack.emplace_back(distr(engine));
            break;
        case EXP:
            EMPTY_PROTECT("exponentiate") {
                m_stack.back() = INT24_C(1) << m_stack.back();
            }
            break;
        case SWP: {
            SIZE_CHECK("swap in", 2);
            size_t i = m_stack.size() - 2;
            std::swap(m_stack[i], m_stack[i + 1]);
            break;
        }
        case THR_E:
            switch (m_ip.dir) {
                case direction::east:
                    m_status = status::terminated;
                    break;
                case direction::west:
                    m_status = status::splitting;
                    break;
                case direction::northeast:
                case direction::southeast:
                    m_status = status::waiting;
                    break;
                default:
                    break;
            }
            break;
        case THR_W:
            switch (m_ip.dir) {
                case direction::west:
                    m_status = status::terminated;
                    break;
                case direction::east:
                    m_status = status::splitting;
                    break;
                case direction::northwest:
                case direction::southwest:
                    m_status = status::waiting;
                    break;
                default:
                    break;
            }
            break;
        case GTM: {
            m_stack.push_back(get_time<trilangle_clock>());
            break;
        }
        case GDT: {
            m_stack.push_back(get_date<trilangle_clock>());
            break;
        }
        case DP2:
            SIZE_CHECK("2-dupe", 2);
            m_stack.push_back(m_stack[m_stack.size() - 2]);
            m_stack.push_back(m_stack[m_stack.size() - 2]);
            break;
        case INVALID_CHAR:
            cout << flush;
            cerr << flush;
            fprintf(
                stderr,
                "Unicode replacement character (U+%04" PRIX32 ") detected in source. Please check encoding.\n",
                static_cast<uint32_t>(op)
            );
            exit(EXIT_FAILURE);
        default:
            cerr << "Unrecognized opcode '";
            print_unichar(op, cerr);
            cerr << "' (at (" << m_ip.coords.first << ", " << m_ip.coords.second << "))\n";
            flush_and_exit(EXIT_FAILURE);
    }
}
