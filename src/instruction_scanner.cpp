#include "instruction_scanner.hh"
#include <list>
#include <unordered_map>

using std::pair;
using std::vector;

void instruction_scanner::build_state() {
    // https://langdev.stackexchange.com/a/659/15

    m_fragments = new vector<vector<instruction>*>{ nullptr };
    // A map of IP -> location in fragments
    std::unordered_map<instruction_pointer, pair<size_t, size_t>> location_map;

    // A collection of fragments that have yet to be visited, corresponding to nulls in m_fragments. The first item of
    // the pair is the index within m_fragments, and the second item is the IP where that fragment begins.
    std::list<pair<size_t, instruction_pointer>> unvisited_fragments = {
        { SIZE_C(0), { { SIZE_C(0), SIZE_C(0) }, direction::southwest } }
    };

    // !empty-back-pop reflects the while let loop in the Rust code in the answer linked above
    while (!unvisited_fragments.empty()) {
        pair<size_t, instruction_pointer> p = unvisited_fragments.back();
        unvisited_fragments.pop_back();
        size_t& index = p.first;
        instruction_pointer& ip = p.second;

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
                fragment->push_back(i);

                if (i.is_exit()) {
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
                        program_walker::branch(ip.dir, op, []() noexcept -> bool {
                            unreachable("is_branch should've returned true");
                        });
                        break;
                    case SKP:
                    case PSI:
                    case PSC:
                        program_walker::advance(ip, m_program->side_length());
                        break;
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
                program_walker::branch(first_ip.dir, op, []() noexcept { return false; });
                program_walker::advance(first_ip, m_program->side_length());

                instruction_pointer second_ip = ip;
                program_walker::branch(second_ip.dir, op, []() noexcept { return true; });
                program_walker::advance(second_ip, m_program->side_length());

                pair<size_t, size_t> first_dest, second_dest;

                // Pushing the first one to the end we're about to read from, and the second one to the far end,
                // minimizes the total number of jumps needed. The second target is going to be pointed to by the BNG or
                // TSP instruction anyways, but the first target only needs to be explicitly named if it's not the
                // next instruction.
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
                    unvisited_fragments.push_front({ m_fragments->size(), second_ip });
                    m_fragments->push_back(nullptr);
                } else {
                    second_dest = loc->second;
                }

                if (op == static_cast<int24_t>(THR_E) || op == static_cast<int24_t>(THR_W)) {
                    fragment->push_back(instruction::spawn_to({ first_dest, second_dest }));
                } else {
                    fragment->push_back(instruction::branch_to({ first_dest, second_dest }));
                }
            } else {
                fragment->push_back(instruction::jump_to(loc->second));
            }
        }

    continue_outer:
        m_fragments->at(index) = fragment;
    }
}
