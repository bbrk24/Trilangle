#include "disassembler.hh"
#include <sstream>

void disassembler::write_state(std::wostream& os) {
    build_state();
    os << L'\t';
    os << static_cast<wchar_t>(m_program.at(m_state_ptr->value.ip.coords.first, m_state_ptr->value.ip.coords.second));
    write(os, *m_state_ptr, false);
    os << std::flush;
}

void disassembler::write(std::wostream& os, const state_element& state, bool label) {
    if (label) {
        os << L"\nlbl" << m_lbl_num++ << L':';
    }

    os << L"\n\t";

    if (!state.first_child) return;

    if (state.first_child->sibling) {
        os << static_cast<wchar_t>(m_program.at(state.first_child->value.ip.coords.first, state.first_child->value.ip.coords.second))
            << L" lbl" << m_lbl_num;

        std::wstringstream wss;
        write(wss, *state.first_child, true);

        os << L", lbl" << m_lbl_num << wss.str()
            << static_cast<wchar_t>(
                m_program.at(state.first_child->sibling->value.ip.coords.first, state.first_child->sibling->value.ip.coords.second)
            );

        write(os, *state.first_child->sibling, true);
    } else {
        os << static_cast<wchar_t>(m_program.at(state.first_child->value.ip.coords.first, state.first_child->value.ip.coords.second));
        write(os, *state.first_child, false);
    }
}

void disassembler::build_state() {
    if (m_state_ptr == nullptr) {
        program_state initial_state{ { { SIZE_C(0), SIZE_C(0) }, direction::southwest } };
        auto p = m_visited.insert(initial_state);

        m_state_ptr = new state_element{ *p.first, nullptr, nullptr };
        build(*m_state_ptr);
    }
}

void disassembler::build(state_element& state) {
    if (m_program.at(state.value.ip.coords.first, state.value.ip.coords.second) == opcode::EXT) {
        return;
    }

    program_state next = state.value;
    program_walker::advance(next.ip, m_program.side_length());

    int24_t op = m_program.at(next.ip.coords.first, next.ip.coords.second);
    bool branched = false;
    switch (op) {
        case MIR_EW: FALLTHROUGH
        case MIR_NESW: FALLTHROUGH
        case MIR_NS: FALLTHROUGH
        case MIR_NWSE:
            program_walker::reflect(next.ip.dir, op);
            break;
        case SKP: FALLTHROUGH
        case PTI: FALLTHROUGH
        case PSC:
            program_walker::advance(next.ip, m_program.side_length());
            break;
        case BNG_E: FALLTHROUGH
        case BNG_NE: FALLTHROUGH
        case BNG_NW: FALLTHROUGH
        case BNG_SE: FALLTHROUGH
        case BNG_SW: FALLTHROUGH
        case BNG_W:
            program_walker::branch(next.ip.dir, op, [&]() {
                branched = true;
                return false;
            });
            break;
        default:
            break;
    }

    auto pair = m_visited.insert(next);
    state.first_child = new state_element{ *pair.first, nullptr, nullptr };

    if (branched) {
        program_state third{ { next.ip.coords, state.value.ip.dir } };
        program_walker::branch(third.ip.dir, op, []() { return true; });

        auto third_pair = m_visited.insert(third);
        auto* el = new state_element{ *third_pair.first, nullptr, nullptr };
        state.first_child->sibling = el;

        if (third_pair.second) {
            build(*el);
        }
    }

    if (pair.second) {
        build(*state.first_child);
    }
}
