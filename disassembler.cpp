#include "disassembler.hh"
#include <sstream>

#define PRINT_NAME(x) case x: os << wss.str() << L ## # x; break

static inline void print_op(
    std::wostream& os,
    disassembler::program_state& state,
    const program& prg,
    long ins_num,
    bool show_branch = false
) {
    int24_t op = prg.at(state.first.coords.first, state.first.coords.second);
    std::wstringstream wss;

    wss << L'\n' << ins_num << L":\t";

    if (state.second == -1L) {
        state.second = ins_num;

        switch (op) {
            case BNG_E: FALLTHROUGH
            case BNG_NE: FALLTHROUGH
            case BNG_NW: FALLTHROUGH
            case BNG_SE: FALLTHROUGH
            case BNG_SW: FALLTHROUGH
            case BNG_W:
                if (show_branch) {
                    os << wss.str() << L"BNG";
                    break;
                }
                FALLTHROUGH
            case SKP: FALLTHROUGH
            case MIR_EW: FALLTHROUGH
            case MIR_NESW: FALLTHROUGH
            case MIR_NS: FALLTHROUGH
            case MIR_NWSE: FALLTHROUGH
            case NOP:
                os << wss.str() << L"NOP";
                break;
            PRINT_NAME(ADD);
            PRINT_NAME(SUB);
            PRINT_NAME(MUL);
            PRINT_NAME(DIV);
            PRINT_NAME(MOD);
            case PSI: {
                auto newip = state.first;
                program_walker::advance(newip, prg.side_length());
                int24_t arg = prg.at(newip.coords.first, newip.coords.second);
                os << wss.str() << L"PSI ." << static_cast<wchar_t>(arg);

                break;
            }
            case PSC: {
                auto newip = state.first;
                program_walker::advance(newip, prg.side_length());
                int24_t arg = prg.at(newip.coords.first, newip.coords.second);
                os << wss.str() << L"PSC '" << static_cast<wchar_t>(arg) << L"' ; ";

                wchar_t buf[9];
                swprintf_s(buf, 9, L"0x%x", static_cast<unsigned int>(arg));
                os << buf;

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
                os << wss.str() << L"Invalid opcode '" << static_cast<wchar_t>(op) << L'\'';
                break;
        }
    } else {
        os << wss.str() << L"JMP " << state.second;
    }
}

void disassembler::write_state(std::wostream& os) {
    build_state();
    print_op(os, m_state_ptr->value, m_program, m_ins_num);
    write(os, *m_state_ptr);
    os << std::flush;
}

void disassembler::write(std::wostream& os, state_element& state) {
    if (!state.first_child) return;

    ++m_ins_num;

    if (state.second_child) {
        print_op(os, state.first_child->value, m_program, m_ins_num, true);

        std::wstringstream wss;
        write(wss, *state.first_child);

        os << L' ' << ++m_ins_num << wss.str();
        print_op(os, state.second_child->value, m_program, m_ins_num);

        write(os, *state.second_child);
    } else {
        print_op(os, state.first_child->value, m_program, m_ins_num);
        write(os, *state.first_child);
    }
}

void disassembler::build_state() {
    if (m_state_ptr == nullptr) {
        program_state initial_state{ { { SIZE_C(0), SIZE_C(0) }, direction::southwest }, -1L };
        auto p = m_visited.insert(initial_state);

        m_state_ptr = new state_element{ *p.first, nullptr, nullptr};
        build(*m_state_ptr);
    }
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
            program_walker::branch(next.dir, op, [&]() {
                branched = true;
                return false;
            });
            break;
        default:
            break;
    }

    auto pair = m_visited.insert({ next, -1L });
    state.first_child = new state_element{ *pair.first, nullptr, nullptr };

    if (branched) {
        instruction_pointer third{ next.coords, state.value.first.dir };

        program_walker::branch(third.dir, op, []() { return true; });

        auto third_pair = m_visited.insert({ third, -1L });
        auto* el = new state_element{ *third_pair.first, nullptr, nullptr };
        state.second_child = el;

        if (third_pair.second) {
            build(*el);
        }
    }

    if (pair.second) {
        build(*state.first_child);
    }
}
