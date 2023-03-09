#include "disassembler.hh"
#include <cinttypes>
#include <cstdlib>
#include <sstream>
#include "output.hh"

// FIXME: This class is held together by an uncomfortable amount of spaghetti. It seems to work, for now, but it's prone
// to error if you look at it funny. It is nontrivial to add new kinds of operations to it. The entire class may have to
// be redesigned from the ground up.

#define PRINT_NAME(x) \
    case x: \
        os << buf << #x "\n"; \
        break

disassembler::~disassembler() noexcept {
    if (m_state_ptr != nullptr) {
        delete m_state_ptr;
    }
}

void disassembler::print_op(
    std::ostream& os,
    disassembler::program_state& state,
    bool show_nops,
    direction from_dir,
    bool show_branch
) {
    int24_t op = m_program->at(state.first.coords.first, state.first.coords.second);

    // Store the label in a temporary buffer, in case it's not actually printed.
    char buf[10];
    snprintf(buf, sizeof buf, "%" PRId32 ":\t", m_ins_num);

    if (state.second == -1) {
        state.second = m_ins_num;

        switch (op) {
            case THR_E:
                switch (from_dir) {
                    case direction::east:
                        os << buf << "TKL\n";
                        break;
                    case direction::west:
                        os << buf << "TSP ";
                        break;
                    case direction::northeast:
                        FALLTHROUGH
                    case direction::southeast:
                        os << buf << "TJN\n";
                        break;
                    case direction::northwest:
                        FALLTHROUGH
                    case direction::southwest:
                        if (show_nops) {
                            os << buf << "NOP\n";
                        }
                        break;
                }
                break;
            case THR_W:
                switch (from_dir) {
                    case direction::west:
                        os << buf << "TKL\n";
                        break;
                    case direction::east:
                        os << buf << "TSP ";
                        break;
                    case direction::northwest:
                        FALLTHROUGH
                    case direction::southwest:
                        os << buf << "TJN\n";
                        break;
                    case direction::northeast:
                        FALLTHROUGH
                    case direction::southeast:
                        if (show_nops) {
                            os << buf << "NOP\n";
                        }
                        break;
                }
                break;
            case BNG_E:
                FALLTHROUGH
            case BNG_NE:
                FALLTHROUGH
            case BNG_NW:
                FALLTHROUGH
            case BNG_SE:
                FALLTHROUGH
            case BNG_SW:
                FALLTHROUGH
            case BNG_W:
                if (show_branch) {
                    os << buf << "BNG ";
                    break;
                }
                FALLTHROUGH
            case SKP:
                FALLTHROUGH
            case MIR_EW:
                FALLTHROUGH
            case MIR_NESW:
                FALLTHROUGH
            case MIR_NS:
                FALLTHROUGH
            case MIR_NWSE:
                FALLTHROUGH
            case NOP:
                if (show_nops) {
                    os << buf << "NOP\n";
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
                program_walker::advance(newip, m_program->side_length());
                int24_t arg = m_program->at(newip.coords.first, newip.coords.second);
                os << buf << "PSI #";
                printunichar(arg, os);
                os << '\n';

                break;
            }
            case PSC: {
                // Print in format "PSC 'A' ; 0x65"
                auto newip = state.first;
                program_walker::advance(newip, m_program->side_length());
                int24_t arg = m_program->at(newip.coords.first, newip.coords.second);
                os << buf << "PSC '";
                printunichar(arg, os);
                os << "' ; 0x";

                snprintf(buf, sizeof buf, "%" PRIx32, static_cast<uint32_t>(arg));
                os << buf << '\n';

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
                os << buf << "Invalid opcode '";
                printunichar(op, os);
                os << "'\n";
                break;
        }
    } else {
        os << buf << "JMP " << state.second << '\n';
    }
}

void disassembler::write_state(std::ostream& os) {
    build_state();
    m_ins_num = 0;

    print_op(os, m_state_ptr->value, !m_flags.hide_nops, direction::southwest);
    write(os, *m_state_ptr);
    os << std::flush;
}

void disassembler::write(std::ostream& os, const state_element& state) {
    if (!state.first_child) {
        return;
    }

    ++m_ins_num;

    if (state.second_child) {
        print_op(os, state.first_child->value, !m_flags.hide_nops, state.value.first.dir, true);

        std::ostringstream ss;
        write(ss, *state.first_child);

        os << (m_ins_num + 1) << '\n' << ss.str();

        write(os, *state.second_child);
    } else {
        print_op(os, state.first_child->value, !m_flags.hide_nops, state.value.first.dir);
        write(os, *state.first_child);
    }
}

void disassembler::build_state() {
    if (m_state_ptr != nullptr) {
        return;
    }

    instruction_pointer initial_ip{ { SIZE_C(0), SIZE_C(0) }, direction::southwest };

    int24_t op = m_program->at(0, 0);
    switch (op) {
        case MIR_EW:
            FALLTHROUGH
        case MIR_NESW:
            FALLTHROUGH
        case MIR_NS:
            FALLTHROUGH
        case MIR_NWSE:
            program_walker::reflect(initial_ip.dir, op);
            break;
        case BNG_E:
            FALLTHROUGH
        case BNG_NE:
            FALLTHROUGH
        case BNG_NW:
            FALLTHROUGH
        case BNG_SE:
            FALLTHROUGH
        case BNG_SW:
            FALLTHROUGH
        case BNG_W:
            program_walker::branch(initial_ip.dir, op, []() -> bool {
                std::cerr << "Error: program starts with branch instruction. Behavior is undefined." << std::endl;
                exit(1);
            });
            break;
        case THR_W:
            std::cerr << "Error: program starts with join instruction. This will wait forever." << std::endl;
            exit(1);
        default:
            break;
    }

    program_state initial_state{ initial_ip, -1 };
    auto p = m_visited.insert(initial_state);

    m_state_ptr = new state_element{ *p.first, nullptr, nullptr };
    build(*m_state_ptr);
}

void disassembler::build(state_element& state) {
    int24_t op = m_program->at(state.value.first.coords.first, state.value.first.coords.second);
    if (op == static_cast<int24_t>(opcode::EXT)) {
        return;
    }

    instruction_pointer next = state.value.first;
    program_walker::advance(next, m_program->side_length());

    if (op == static_cast<int24_t>(opcode::PSC) || op == static_cast<int24_t>(opcode::PSI)
        || op == static_cast<int24_t>(opcode::SKP)) {
        program_walker::advance(next, m_program->side_length());
    }

    op = m_program->at(next.coords.first, next.coords.second);

    if (op == static_cast<int24_t>(opcode::THR_W) && state.value.first.dir == direction::west) {
        auto pair = m_visited.insert({ { next.coords, direction::east }, -1 });
        state.first_child = new state_element{ *pair.first, nullptr, nullptr };
        return;
    } else if (op == static_cast<int24_t>(opcode::THR_E) && state.value.first.dir == direction::east) {
        auto pair = m_visited.insert({ { next.coords, direction::west }, -1 });
        state.first_child = new state_element{ *pair.first, nullptr, nullptr };
        return;
    }

    bool branched = false;
    switch (op) {
        case MIR_EW:
            FALLTHROUGH
        case MIR_NESW:
            FALLTHROUGH
        case MIR_NS:
            FALLTHROUGH
        case MIR_NWSE:
            program_walker::reflect(next.dir, op);
            break;
        case THR_E:
            FALLTHROUGH
        case THR_W:
            FALLTHROUGH
        case BNG_E:
            FALLTHROUGH
        case BNG_NE:
            FALLTHROUGH
        case BNG_NW:
            FALLTHROUGH
        case BNG_SE:
            FALLTHROUGH
        case BNG_SW:
            FALLTHROUGH
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
