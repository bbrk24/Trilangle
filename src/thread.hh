#pragma once

#include <cinttypes>
#include <cstdlib>
#include <random>
#include "input.hh"
#include "program_walker.hh"
#include "time.hh"

extern unsigned long thread_count;

#ifndef TRILANGLE_CLOCK
#define TRILANGLE_CLOCK std::chrono::system_clock
#endif

using trilangle_clock = TRILANGLE_CLOCK;

extern "C" void send_debug_info(
    unsigned long thread_number,
    const int24_t* stack,
    size_t stack_depth,
    size_t y,
    size_t x,
    CONST_C_STR instruction
);

template<class ProgramHolder>
class thread {
    template<class>
    friend class interpreter;
public:
    enum class status : char {
        active,      // Currently executing code
        idle,        // Inactive for a single tick due to e.g. a "skip" operation
        waiting,     // Waiting for a second thread to merge with
        splitting,   // Arrived at a thread split and waiting to be split in two
        terminated,  // The thread is no longer executing
    };

    // Currently only called with T = std::vector<int24_t> and T = const std::vector<int24_t>&.
    // Generic to allow both move- and copy-construction.
    template<typename T>
    thread(const thread<program_walker>& other, direction d, T&& stack) noexcept :
        m_program_holder(other.m_program_holder),
        m_stack(std::forward<T>(stack)),
        m_ip{ other.m_ip.coords, d },
        m_status(status::active),
        m_flags(other.m_flags),
        m_number(++thread_count) {
        advance();
    }

    inline thread(const thread<ProgramHolder>& other, std::vector<int24_t>&& stack) noexcept :
        m_program_holder(other.m_program_holder),
        m_stack(std::forward<std::vector<int24_t>&&>(stack)),
        m_ip{ other.m_ip },
        m_status(status::active),
        m_flags(other.m_flags),
        m_number(++thread_count) {
        advance();
    }

    inline thread(const thread<program_walker>& other, direction d) noexcept : thread(other, d, other.m_stack) {}

#define EMPTY_PROTECT(name) \
    if (m_stack.empty() && m_flags.warnings) UNLIKELY { \
        std::cerr << "Warning: Attempt to " name " empty stack.\n"; \
    } else

#define SIZE_CHECK(name, count) \
    if (m_flags.warnings && m_stack.size() < (count)) UNLIKELY \
    cerr << "Warning: Attempt to " name " stack with fewer than " << (count) << " elements.\n"

    inline void tick() {
        using std::cerr;
        using std::cout;
        using std::flush;
        using std::pair;
        using operation = instruction::operation;

        static std::default_random_engine engine((std::random_device())());
        static std::uniform_int_distribution<int32_t> distr(INT24_MIN, INT24_MAX);

        // The operation currently being executed
        instruction instr = m_program_holder->at(m_ip);
        operation op = instr.get_op();

        // The web interface needs every thread to send debug info. The CLI expects only active threads to do so.
#ifdef __EMSCRIPTEN__
        if (m_flags.debug) {
            pair<size_t, size_t> coords = m_program_holder->get_coords(m_ip);
            send_debug_info(
                m_number,
                m_flags.show_stack ? m_stack.data() : nullptr,
                m_stack.size(),
                coords.first,
                coords.second,
                m_program_holder->raw_at(m_ip).c_str()
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
            pair<size_t, size_t> coords = m_program_holder->get_coords(m_ip);
            send_debug_info(
                m_number,
                m_flags.show_stack ? m_stack.data() : nullptr,
                m_stack.size(),
                coords.first,
                coords.second,
                m_program_holder->raw_at(m_ip).c_str()
            );
        }
#endif

        // ...yeah
        switch (op) {
            case operation::BNG:
            case operation::NOP:
            case operation::JMP:
                break;
            case operation::ADD: {
                SIZE_CHECK("add from", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();

                auto [overflow, value] = m_stack.back().add_with_overflow(top);

                if (m_flags.warnings && overflow) UNLIKELY {
                    cerr << "Warning: Overflow on addition/subtraction is undefined behavior.\n";
                }

                m_stack.back() = value;

                break;
            }
            case operation::SUB: {
                SIZE_CHECK("subtract from", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();

                auto [overflow, value] = m_stack.back().subtract_with_overflow(top);

                if (m_flags.warnings && overflow) UNLIKELY {
                    cerr << "Warning: Overflow on addition/subtraction is undefined behavior.\n";
                }

                m_stack.back() = value;

                break;
            }
            case operation::MUL: {
                SIZE_CHECK("multiply from", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();

                auto [overflow, value] = m_stack.back().multiply_with_overflow(top);

                if (m_flags.warnings && overflow) UNLIKELY {
                    cerr << "Warning: Overflow on multiplication is undefined behavior.\n";
                }

                m_stack.back() = value;

                break;
            }
            case operation::DIV: {
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
            case operation::UDV: {
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
                int24_t second = m_stack.back();

                uint32_t unsigned_first = static_cast<uint32_t>(top) & 0x00ff'ffffU;
                uint32_t unsigned_second = static_cast<uint32_t>(second) & 0x00ff'ffffU;

                m_stack.back() = static_cast<int24_t>(unsigned_second / unsigned_first);
                break;
            }
            case operation::MOD: {
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
            case operation::PSC: {
                m_status = status::idle;
                int24_t next = instr.get_arg().number;
                m_stack.push_back(next);
                break;
            }
            case operation::PSI: {
                m_status = status::idle;
                int24_t next = instr.get_arg().number;

                if (m_flags.warnings) {
                    if (next < (int24_t)'0' || next > (int24_t)'9') UNLIKELY {
                        cerr << "Warning: Pushing non-ASCII-decimal number with " << static_cast<char>(opcode::PSI)
                             << " is implementation-defined behavior.\n";
                    }
                }

                m_stack.push_back(next - (int24_t)'0');
                break;
            }
            case operation::POP:
                EMPTY_PROTECT("pop from") {
                    m_stack.pop_back();
                }
                break;
            case operation::EXT:
                thread<ProgramHolder>::flush_and_exit(EXIT_SUCCESS);
            case operation::INC:
                EMPTY_PROTECT("increment") {
                    ++m_stack.back();
                }
                break;
            case operation::DEC:
                EMPTY_PROTECT("decrement") {
                    --m_stack.back();
                }
                break;
            case operation::AND: {
                SIZE_CHECK("bitwise and", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() &= top;
                break;
            }
            case operation::IOR: {
                SIZE_CHECK("bitwise or", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() |= top;
                break;
            }
            case operation::XOR: {
                SIZE_CHECK("bitwise xor", 2);
                int24_t top = m_stack.back();
                m_stack.pop_back();
                m_stack.back() ^= top;
                break;
            }
            case operation::NOT:
                EMPTY_PROTECT("complement") {
                    m_stack.back() = ~m_stack.back();
                }
                break;
            case operation::GTC:
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
            case operation::PTC:
                EMPTY_PROTECT("print from") {
                    bool should_print = true;

                    if (m_stack.back() < INT24_C(0)) UNLIKELY {
                        if (m_flags.warnings) {
                            cerr << "Warning: Attempt to print character with negative value.\n";
                            should_print = false;
                        }
                        if (m_flags.pipekill) {
                            thread<ProgramHolder>::flush_and_exit(EXIT_FAILURE);
                        }
                    }

                    if (should_print) {
                        if (m_flags.assume_ascii) {
                            int24_t back = m_stack.back();
                            if (m_flags.warnings && (back & INT24_C(0x7f)) != back) UNLIKELY {
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
            case operation::GTI: {
                int32_t i = -1;

                while (!(feof(stdin) || scanf("%" SCNi32, &i))) {
                    [[maybe_unused]] int _ = getchar();
                }

                m_stack.emplace_back(i);
                break;
            }
            case operation::PTI:
                EMPTY_PROTECT("print from") {
                    cout << m_stack.back() << '\n';
                }

                if (m_flags.pipekill && ferror(stdout)) {
                    cerr << flush;
                    exit(EXIT_SUCCESS);
                }
                break;
            case operation::PTU:
                EMPTY_PROTECT("print from") {
                    cout << (static_cast<uint32_t>(m_stack.back()) & 0x00ff'ffffU) << '\n';
                }

                if (m_flags.pipekill && ferror(stdout)) {
                    cerr << flush;
                    exit(EXIT_SUCCESS);
                }
                break;
            case operation::IDX: {
                if (m_flags.warnings && m_stack.empty()) UNLIKELY {
                    cerr << "Warning: Attempt to read index from empty stack.\n";
                    break;
                }

                size_t index = static_cast<size_t>(m_stack.back()) & SIZE_C(0x00ff'ffff);
                m_stack.pop_back();

                if (m_flags.warnings && m_stack.size() < index + 1) UNLIKELY {
                    cerr << "Warning: Attempt to index out of stack bounds (size = " << m_stack.size()
                         << ", index = " << index << ")\n";
                }

                size_t i = m_stack.size() - index - 1;
                m_stack.push_back(m_stack[i]);

                break;
            }
            case operation::DUP:
                EMPTY_PROTECT("duplicate") {
                    m_stack.push_back(m_stack.back());
                }
                break;
            case operation::RND:
                m_stack.emplace_back(distr(engine));
                break;
            case operation::EXP:
                EMPTY_PROTECT("exponentiate") {
                    m_stack.back() = INT24_C(1) << m_stack.back();
                }
                break;
            case operation::SWP: {
                SIZE_CHECK("swap in", 2);
                size_t i = m_stack.size() - 2;
                std::swap(m_stack[i], m_stack[i + 1]);
                break;
            }
            case operation::TSP:
                m_status = status::splitting;
                break;
            case operation::TJN:
                m_status = status::waiting;
                break;
            case operation::TKL:
                m_status = status::terminated;
                break;
            case operation::GTM: {
                m_stack.push_back(get_time<trilangle_clock>());
                break;
            }
            case operation::GDT: {
                m_stack.push_back(get_date<trilangle_clock>());
                break;
            }
            case operation::DP2:
                SIZE_CHECK("2-dupe", 2);
                m_stack.push_back(m_stack[m_stack.size() - 2]);
                m_stack.push_back(m_stack[m_stack.size() - 2]);
                break;
// Just because they're not defined enum constants doesn't mean the input program won't have them
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wswitch"
#elif REALLY_MSVC
#pragma warning(push)
#pragma warning(disable : 4063)
#endif
            case static_cast<operation>(SKP):
                m_status = status::idle;
                break;
            case static_cast<operation>(INVALID_CHAR):
                cout << flush;
                cerr << flush;
                fprintf(
                    stderr,
                    "Encoding error detected, or U+%04" PRIX32 " present in source.\n",
                    static_cast<uint32_t>(op)
                );
                exit(EXIT_FAILURE);
            default: {
                cerr << "Unrecognized opcode '";
                print_unichar(static_cast<int24_t>(op), cerr);
                pair<size_t, size_t> coords = m_program_holder->get_coords(m_ip);
                cerr << "' (at (" << coords.first << ", " << coords.second << "))\n";
                thread<ProgramHolder>::flush_and_exit(EXIT_FAILURE);
            }
#ifdef __clang__
#pragma clang diagnostic pop
#elif REALLY_MSVC
#pragma warning(pop)
#endif
        }
    }

#undef SIZE_CHECK
protected:
    inline thread(ProgramHolder* ph, flags f) noexcept :
        m_program_holder(ph), m_stack(), m_ip(), m_status(status::active), m_flags(f), m_number(thread_count++) {}

    constexpr void advance() noexcept {
        m_program_holder->advance(m_ip, [&]() noexcept {
            EMPTY_PROTECT("branch on") {}
            return m_stack.back() < INT24_C(0);
        });
    }
#undef EMPTY_PROTECT

    ProgramHolder* m_program_holder;
    std::vector<int24_t> m_stack;
    NO_UNIQUE_ADDRESS typename ProgramHolder::IP m_ip;
    status m_status;
    flags m_flags;
private:
    [[noreturn]] static inline void flush_and_exit(int code) {
        std::cout << std::flush;
        std::cerr << std::flush;
#ifdef __EMSCRIPTEN__
        // https://github.com/bbrk24/Trilangle/issues/4
        while (!feof(stdin)) {
            [[maybe_unused]] int _ = getchar();
        }
#endif
        exit(code);
    }

    unsigned long m_number;
};
