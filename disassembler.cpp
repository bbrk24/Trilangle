#include "disassembler.hh"
#include <sstream>
#include <iostream>
#include <cstdlib>
#include <cinttypes>

#define PRINT_NAME(x) case x: os << buf << L ## # x; printed_first_line = true; break

static void print_op(
    std::wostream& os,
    disassembler::program_state& state,
    const program& prg,
    int32_t ins_num,
    bool show_nops,
    bool show_branch = false
) {
    static bool printed_first_line = false;

    int24_t op = prg.at(state.first.coords.first, state.first.coords.second);

    // Store the label in a temporary buffer, in case it's not actually printed.
    wchar_t buf[10];
    swprintf(
        buf,
        sizeof buf / sizeof (wchar_t),
        printed_first_line
            ? L"\n%" PRId32 ":\t"
            :   L"%" PRId32 ":\t",
        ins_num
    );

    if (state.second == -1) {
        state.second = ins_num;

        switch (op) {
            case BNG_E: FALLTHROUGH
            case BNG_NE: FALLTHROUGH
            case BNG_NW: FALLTHROUGH
            case BNG_SE: FALLTHROUGH
            case BNG_SW: FALLTHROUGH
            case BNG_W:
                if (show_branch) {
                    os << buf << L"BNG";
                    printed_first_line = true;
                    break;
                }
                FALLTHROUGH
            case SKP: FALLTHROUGH
            case MIR_EW: FALLTHROUGH
            case MIR_NESW: FALLTHROUGH
            case MIR_NS: FALLTHROUGH
            case MIR_NWSE: FALLTHROUGH
            case NOP:
                if (show_nops) {
                    os << buf << L"NOP";
                    printed_first_line = true;
                }
                break;
            PRINT_NAME(ADD);
            PRINT_NAME(SUB);
            PRINT_NAME(MUL);
            PRINT_NAME(DIV);
            PRINT_NAME(MOD);
            case PSI: {
                // Print in format "PSI #3"
                auto newip = state.first;
                program_walker::advance(newip, prg.side_length());
                int24_t arg = prg.at(newip.coords.first, newip.coords.second);
                os << buf << L"PSI #" << static_cast<wchar_t>(arg);

                printed_first_line = true;
                break;
            }
            case PSC: {
                // Print in format "PSC 'A' ; 0x65"
                auto newip = state.first;
                program_walker::advance(newip, prg.side_length());
                int24_t arg = prg.at(newip.coords.first, newip.coords.second);
                os << buf << L"PSC '" << static_cast<wchar_t>(arg) << L"' ; 0x";

                swprintf(buf, sizeof buf / sizeof (wchar_t), L"%" PRIx32, static_cast<uint32_t>(arg));
                os << buf;

                printed_first_line = true;
                break;
            }
            PRINT_NAME(POP);
            PRINT_NAME(EXT);
            PRINT_NAME(INC);
            PRINT_NAME(DEC);
            PRINT_NAME(AND);
            PRINT_NAME(IOR);
            PRINT_NAME(XOR);
            PRINT_NAME(NOT);
            PRINT_NAME(GTC);
            PRINT_NAME(PTC);
            PRINT_NAME(GTI);
            PRINT_NAME(PTI);
            PRINT_NAME(IDX);
            PRINT_NAME(DUP);
            PRINT_NAME(RND);
            PRINT_NAME(EXP);
            PRINT_NAME(SWP);
            default:
                os << buf << L"Invalid opcode '" << static_cast<wchar_t>(op) << L'\'';
                printed_first_line = true;
                break;
        }
    } else {
        os << buf << L"JMP " << state.second;
        printed_first_line = true;
    }
}

void disassembler::write_state(std::wostream& os) {
    build_state();
    m_ins_num = 0;
    print_op(os, m_state_ptr->value, m_program, m_ins_num, !m_flags.hide_nops);
    write(os, *m_state_ptr);
    os << std::endl;
}

void disassembler::write(std::wostream& os, state_element& state) {
    if (!state.first_child) return;

    ++m_ins_num;

    if (state.second_child) {
        print_op(os, state.first_child->value, m_program, m_ins_num, !m_flags.hide_nops, true);

        std::wostringstream wss;
        write(wss, *state.first_child);

        os << L' ' << (m_ins_num + 1) << wss.str();

        write(os, *state.second_child);
    } else {
        print_op(os, state.first_child->value, m_program, m_ins_num, !m_flags.hide_nops);
        write(os, *state.first_child);
    }
}

void disassembler::build_state() {
    if (m_state_ptr != nullptr) {
        return;
    }

    instruction_pointer initial_ip{ { SIZE_C(0), SIZE_C(0) }, direction::southwest };

    int24_t op = m_program.at(0, 0);
    switch (op) {
        case MIR_EW: FALLTHROUGH
        case MIR_NESW: FALLTHROUGH
        case MIR_NS: FALLTHROUGH
        case MIR_NWSE:
            program_walker::reflect(initial_ip.dir, op);
            break;
        case BNG_E: FALLTHROUGH
        case BNG_NE: FALLTHROUGH
        case BNG_NW: FALLTHROUGH
        case BNG_SE: FALLTHROUGH
        case BNG_SW: FALLTHROUGH
        case BNG_W:
            program_walker::branch(initial_ip.dir, op, []() -> bool {
                std::cerr << "Error: program starts with branch instruction. Behavior is undefined." << std::endl;
                exit(1);
            });
            break;
        default:
            break;
    }

    program_state initial_state{ initial_ip, -1 };
    auto p = m_visited.insert(initial_state);

    m_state_ptr = new state_element{ *p.first, nullptr, nullptr };
    build(*m_state_ptr);
}

void disassembler::build(state_element& state) {
    int24_t op = m_program.at(state.value.first.coords.first, state.value.first.coords.second);
    if (op == opcode::EXT) {
        return;
    }

    instruction_pointer next = state.value.first;
    program_walker::advance(next, m_program.side_length());

    if (op == opcode::PSC || op == opcode::PSI || op == opcode::SKP) {
        program_walker::advance(next, m_program.side_length());
    }

    op = m_program.at(next.coords.first, next.coords.second);
    bool branched = false;
    switch (op) {
        case MIR_EW: FALLTHROUGH
        case MIR_NESW: FALLTHROUGH
        case MIR_NS: FALLTHROUGH
        case MIR_NWSE:
            program_walker::reflect(next.dir, op);
            break;
        case BNG_E: FALLTHROUGH
        case BNG_NE: FALLTHROUGH
        case BNG_NW: FALLTHROUGH
        case BNG_SE: FALLTHROUGH
        case BNG_SW: FALLTHROUGH
        case BNG_W:
            program_walker::branch(next.dir, op, [&]() NOEXCEPT_T {
                branched = true;
                return false;
            });
            break;
        default:
            break;
    }

    auto pair = m_visited.insert({ next, -1 });
    state.first_child = new state_element{ *pair.first, nullptr, nullptr };

    if (pair.second) {
        build(*state.first_child);
    }

    if (branched) {
        instruction_pointer third{ next.coords, state.value.first.dir };

        program_walker::branch(third.dir, op, []() NOEXCEPT_T { return true; });

        auto third_pair = m_visited.insert({ third, -1 });
        auto* el = new state_element{ *third_pair.first, nullptr, nullptr };
        state.second_child = el;

        if (third_pair.second) {
            build(*el);
        }
    }
}
