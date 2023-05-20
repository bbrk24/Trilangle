#include "disassembler.hh"
#include <unordered_map>

using std::pair;
using std::vector;

disassembler::~disassembler() {
    if (m_fragments == nullptr) {
        return;
    }
    for (vector<instruction>* frag : *m_fragments) {
        delete frag;
    }
    delete m_fragments;
}

void disassembler::write_state(std::ostream& os) {
    if (m_fragments == nullptr) {
        build_state();
    }

    for (size_t i = 0; i < m_fragments->size(); ++i) {
        const vector<instruction>& frag = *m_fragments->at(i);
        for (size_t j = 0; j < frag.size(); ++j) {
            // skip NOPs if requested
            const instruction& ins = frag[j];
            if (m_flags.hide_nops && ins.is_nop()) {
                continue;
            }

            // write out the label
            os << i << '.' << j << ":\t";
            // write out the instruction name
            os << ins.to_str();

            // if it's a branch followed by a jump, write that
            const pair<size_t, size_t>* next = ins.first_if_branch();
            if (next != nullptr && (next->first != i + 1 || next->second != 0)) {
                os << "\n\tJMP " << next->first << '.' << next->second;
            }

            // write the newline
            os << '\n';
        }
    }

    // Flush the buffer, if applicable
    os << std::flush;
}

void disassembler::build_state() {
    // https://languagedesign.stackexchange.com/a/659/15

    m_fragments = new vector<vector<instruction>*>{ nullptr };
    // A map of IP -> location in fragments
    std::unordered_map<instruction_pointer, pair<size_t, size_t>> location_map;

    instruction_pointer ip;
    size_t index;

    // A collection of fragments that have yet to be visited, corresponding to nulls in m_fragments. The first item of
    // the pair is the index within m_fragments, and the second item is the IP where that fragment begins.
    vector<pair<size_t, instruction_pointer>> unvisited_fragments = {
        { SIZE_C(0), { { SIZE_C(0), SIZE_C(0) }, direction::southwest } }
    };

    // !empty-back-pop reflects the while let loop in the Rust code in the answer linked above
    while (!unvisited_fragments.empty()) {
        pair<size_t, instruction_pointer> p = unvisited_fragments.back();
        unvisited_fragments.pop_back();
        index = p.first;
        ip = p.second;

        auto* fragment = new vector<instruction>();

        int24_t op = m_program->at(ip.coords.first, ip.coords.second);
        while (!is_branch(op, ip.dir)) {
            auto loc = location_map.find(ip);
            if (loc == location_map.end()) {
                // Special-case TJN
                // Unlike all other instructions, if TJN at the same location is hit from different directions, it
                // MUST be emitted into the assembly exactly once.
                instruction i(ip, *m_program);
                if (i.is_join()) {
                    // flip the north/south bit on the direction and check again
                    loc = location_map.find({ ip.coords, static_cast<direction>(static_cast<char>(ip.dir) ^ 0b100) });
                    if (loc != location_map.end()) {
                        location_map.insert({ ip, loc->second });

                        // jump to the other one
                        fragment->push_back(instruction::jump_to(loc->second));
                        // Yeah, yeah, goto is bad practice. You look at this loop and tell me you'd rather do it
                        // another way.
                        goto continue_outer;
                    }
                }

                location_map.insert({ ip, { index, fragment->size() } });
                fragment->push_back(std::move(i));

                if (fragment->back().is_exit()) {
                    // doesn't end in a jump nor a branch
                    goto continue_outer;
                }

                switch (op) {
                    case MIR_EW:
                    case MIR_NESW:
                    case MIR_NS:
                    case MIR_NWSE:
                        program_walker::reflect(ip.dir, op);
                        break;
                    case BNG_E:
                    case BNG_W:
                    case BNG_NE:
                    case BNG_NW:
                    case BNG_SE:
                    case BNG_SW:
                    case THR_E:
                    case THR_W:
                        program_walker::branch(ip.dir, op, []() NOEXCEPT_T -> bool {
                            unreachable("is_branch should've returned true");
                        });
                        break;
                    case SKP:
                    case PSI:
                    case PSC:
                        program_walker::advance(ip, m_program->side_length());
                    default:
                        break;
                }

                program_walker::advance(ip, m_program->side_length());
            } else {
                fragment->push_back(instruction::jump_to(loc->second));
                goto continue_outer;
            }

            op = m_program->at(ip.coords.first, ip.coords.second);
        }

        // Putting all the variables in this scope so that way continue_outer can't see them
        {
            // Determine whether to emit a jump or branch. If it's a branch, determine what the targets are.
            auto loc = location_map.find(ip);
            if (loc == location_map.end()) {
                location_map.insert({ ip, { index, fragment->size() } });

                // BNG requires that its first target go right and its second go left. TSP doesn't really care.
                instruction_pointer first_ip = ip;
                program_walker::branch(first_ip.dir, op, []() NOEXCEPT_T { return false; });
                program_walker::advance(first_ip, m_program->side_length());

                instruction_pointer second_ip = ip;
                program_walker::branch(second_ip.dir, op, []() NOEXCEPT_T { return true; });
                program_walker::advance(second_ip, m_program->side_length());

                pair<size_t, size_t> first_dest, second_dest;

                loc = location_map.find(first_ip);
                if (loc == location_map.end()) {
                    first_dest = { m_fragments->size(), SIZE_C(0) };
                    unvisited_fragments.push_back({ m_fragments->size(), first_ip });
                    m_fragments->push_back(nullptr);
                } else {
                    first_dest = loc->second;
                }

                loc = location_map.find(second_ip);
                if (loc == location_map.end()) {
                    second_dest = { m_fragments->size(), SIZE_C(0) };
                    unvisited_fragments.push_back({ m_fragments->size(), second_ip });
                    m_fragments->push_back(nullptr);
                } else {
                    second_dest = loc->second;
                }

                fragment->push_back(instruction::branch_to({ first_dest, second_dest }));
            } else {
                fragment->push_back(instruction::jump_to(loc->second));
            }
        }

    continue_outer:
        m_fragments->at(index) = fragment;
    }
}
