#pragma once

#include <sstream>
#include "any_program_holder.hh"
#include "output.hh"

enum class direction : char {
    // Bitfield representation: { north, !(north || south), east }. This is designed so the default direction
    // (southwest) is 0 and using this bitfield rather than consecutive values reduces binary size slightly.

    southwest = 0b000,
    west = 0b010,
    northwest = 0b100,
    northeast = 0b101,
    east = 0b011,
    southeast = 0b001,
};

constexpr bool is_branch(int24_t op, direction dir) noexcept {
    switch (op) {
        case BNG_E:
        case THR_E:
            return dir == direction::west;
        case BNG_W:
        case THR_W:
            return dir == direction::east;
        case BNG_NE:
            return dir == direction::southwest;
        case BNG_NW:
            return dir == direction::southeast;
        case BNG_SE:
            return dir == direction::northwest;
        case BNG_SW:
            return dir == direction::northeast;
        default:
            return false;
    }
}

struct instruction_pointer {
    std::pair<size_t, size_t> coords;
    direction dir;

    constexpr bool operator==(const instruction_pointer& rhs) const noexcept {
        return this->coords == rhs.coords && this->dir == rhs.dir;
    }
};

class program_walker : public any_program_holder<instruction_pointer> {
public:
    using IP = instruction_pointer;

    constexpr program_walker(NONNULL_PTR(const program) p) noexcept : m_program(p) {}

    inline void advance(IP& ip, std::function<bool()> go_left) {
        int24_t op = m_program->at(ip.coords.first, ip.coords.second);
        switch (op) {
            case MIR_EW:
            case MIR_NESW:
            case MIR_NS:
            case MIR_NWSE:
                program_walker::reflect(ip.dir, op);
                break;
            case BNG_E:
            case BNG_NE:
            case BNG_NW:
            case BNG_SE:
            case BNG_SW:
            case BNG_W:
            case THR_E:
            case THR_W:
                program_walker::branch(ip.dir, op, go_left);
                break;
            case SKP:
            case PSC:
            case PSI:
                program_walker::advance(ip, m_program->side_length());
                break;
            default:
                break;
        }
        program_walker::advance(ip, m_program->side_length());
    }

    inline instruction at(const IP& ip) noexcept {
        int24_t op = m_program->at(ip.coords.first, ip.coords.second);
        if (is_branch(op, ip.dir)) {
            // The arguments here aren't used. This needs to return `instruction` rather than `operation` in order to
            // convey the argument for PSI/PSC, but the arguments for branches aren't used by the threads. While I could
            // determine the locations that they would branch to, pair<size_t> is smaller than IP, so that wouldn't even
            // fit in the arguments. Instead, the arguments have to be hardcoded with some value, like 0.
            if (op == static_cast<int24_t>(THR_E) || op == static_cast<int24_t>(THR_W)) {
                return instruction::spawn_to({ { SIZE_C(0), SIZE_C(0) }, { SIZE_C(0), SIZE_C(0) } });
            } else {
                return instruction::branch_to({ { SIZE_C(0), SIZE_C(0) }, { SIZE_C(0), SIZE_C(0) } });
            }
        } else if (op == static_cast<int24_t>(SKP)) {
            return instruction{ static_cast<instruction::operation>(SKP), instruction::argument() };
        } else {
            return instruction(ip, *m_program);
        }
    }


    inline std::pair<size_t, size_t> get_coords(const IP& ip) const { return ip.coords; }

    inline std::string raw_at(const IP& ip) {
        std::ostringstream oss;
        print_unichar(m_program->at(ip.coords.first, ip.coords.second), oss);
        return oss.str();
    }

    // Advance the IP one step.
    static constexpr void advance(instruction_pointer& ip, size_t program_size) noexcept {
        // I don't remember how this works, it just does.
        switch (ip.dir) {
            case direction::southwest:
                if (++ip.coords.first == program_size) {
                    ip.coords.second = (ip.coords.second + 1) % program_size;
                    ip.coords.first = ip.coords.second;
                }
                break;
            case direction::west:
                if (ip.coords.second == 0) {
                    if (++ip.coords.first == program_size) {
                        ip.coords.first = 0;
                    } else {
                        ip.coords.second = ip.coords.first;
                    }
                } else {
                    --ip.coords.second;
                }
                break;
            case direction::northwest:
                if (ip.coords.second == 0) {
                    if (ip.coords.first == program_size - 1) {
                        ip.coords.second = program_size - 1;
                    } else {
                        ip.coords.second = program_size - ip.coords.first - 2;
                        ip.coords.first = program_size - 1;
                    }
                } else {
                    --ip.coords.first;
                    --ip.coords.second;
                }
                break;
            case direction::northeast:
                if (ip.coords.first == 0 || ip.coords.second >= ip.coords.first) {
                    ip.coords.first = program_size - 1;
                    if (ip.coords.second == 0) {
                        ip.coords.second = program_size - 1;
                    } else {
                        --ip.coords.second;
                    }
                } else {
                    --ip.coords.first;
                }
                break;
            case direction::east:
                if (++ip.coords.second > ip.coords.first) {
                    if (ip.coords.first == 0) {
                        ip.coords.first = program_size - 1;
                    } else {
                        --ip.coords.first;
                    }
                    ip.coords.second = 0;
                }
                break;
            case direction::southeast:
                ++ip.coords.second;
                if (++ip.coords.first == program_size) {
                    if (ip.coords.second < program_size) {
                        ip.coords.first = program_size - ip.coords.second - 1;
                    } else {
                        ip.coords.first = program_size - 1;
                    }
                    ip.coords.second = 0;
                }
                break;
        }
    }

#if REALLY_MSVC
#pragma warning(push)
#pragma warning(disable : 4702)
    // C4702 is unreachable code, which it complains about when constant-folding the template instantiation
#endif

    // Move the IP according to the branch instruction.
    template<typename FuncType>
    static void branch(direction& dir, int24_t bng, FuncType go_left) noexcept(noexcept(go_left())) {
        switch (bng) {
            case THR_E:
                switch (dir) {
                    case direction::west:
                        if (go_left()) {
                            dir = direction::southwest;
                        } else {
                            dir = direction::northwest;
                        }
                        break;
                    case direction::northeast:
                    case direction::southeast:
                        dir = direction::east;
                        break;
                    default:
                        break;
                }
                break;
            case BNG_E:
                switch (dir) {
                    case direction::west:
                        if (go_left()) {
                            dir = direction::southwest;
                        } else {
                            dir = direction::northwest;
                        }
                        break;
                    case direction::northeast:
                    case direction::southeast:
                        dir = direction::east;
                        break;
                    case direction::east:
                        dir = direction::west;
                        break;
                    case direction::southwest:
                        dir = direction::northeast;
                        break;
                    case direction::northwest:
                        dir = direction::southeast;
                        break;
                }
                break;
            case THR_W:
                switch (dir) {
                    case direction::east:
                        if (go_left()) {
                            dir = direction::northeast;
                        } else {
                            dir = direction::southeast;
                        }
                        break;
                    case direction::northwest:
                    case direction::southwest:
                        dir = direction::west;
                        break;
                    default:
                        break;
                }
                break;
            case BNG_W:
                switch (dir) {
                    case direction::east:
                        if (go_left()) {
                            dir = direction::northeast;
                        } else {
                            dir = direction::southeast;
                        }
                        break;
                    case direction::northwest:
                    case direction::southwest:
                        dir = direction::west;
                        break;
                    case direction::west:
                        dir = direction::east;
                        break;
                    case direction::southeast:
                        dir = direction::northwest;
                        break;
                    case direction::northeast:
                        dir = direction::southwest;
                        break;
                }
                break;
            case BNG_NE:
                switch (dir) {
                    case direction::southwest:
                        if (go_left()) {
                            dir = direction::southeast;
                        } else {
                            dir = direction::west;
                        }
                        break;
                    case direction::northwest:
                    case direction::east:
                        dir = direction::northeast;
                        break;
                    case direction::northeast:
                        dir = direction::southwest;
                        break;
                    case direction::west:
                        dir = direction::east;
                        break;
                    case direction::southeast:
                        dir = direction::northwest;
                        break;
                }
                break;
            case BNG_SW:
                switch (dir) {
                    case direction::northeast:
                        if (go_left()) {
                            dir = direction::northwest;
                        } else {
                            dir = direction::east;
                        }
                        break;
                    case direction::southeast:
                    case direction::west:
                        dir = direction::southwest;
                        break;
                    case direction::northwest:
                        dir = direction::southeast;
                        break;
                    case direction::east:
                        dir = direction::west;
                        break;
                    case direction::southwest:
                        dir = direction::northeast;
                        break;
                }
                break;
            case BNG_NW:
                switch (dir) {
                    case direction::southeast:
                        if (go_left()) {
                            dir = direction::east;
                        } else {
                            dir = direction::southwest;
                        }
                        break;
                    case direction::northeast:
                    case direction::west:
                        dir = direction::northwest;
                        break;
                    case direction::northwest:
                        dir = direction::southeast;
                        break;
                    case direction::east:
                        dir = direction::west;
                        break;
                    case direction::southwest:
                        dir = direction::northeast;
                        break;
                }
                break;
            case BNG_SE:
                switch (dir) {
                    case direction::northwest:
                        if (go_left()) {
                            dir = direction::west;
                        } else {
                            dir = direction::northeast;
                        }
                        break;
                    case direction::southwest:
                    case direction::east:
                        dir = direction::southeast;
                        break;
                    case direction::northeast:
                        dir = direction::southwest;
                        break;
                    case direction::west:
                        dir = direction::east;
                        break;
                    case direction::southeast:
                        dir = direction::northwest;
                        break;
                }
                break;
            default:
                unreachable("program_walker::branch() should only be passed a branch");
        }
    }

#if REALLY_MSVC
#pragma warning(pop)
#endif
protected:
    NONNULL_PTR(const program) m_program;

    // Reflect the IP according to the mirror.
    static inline void reflect(direction& dir, int24_t mir) noexcept {
        switch (mir) {
            case MIR_EW:
                switch (dir) {
                    case direction::southwest:
                        dir = direction::northwest;
                        break;
                    case direction::northwest:
                        dir = direction::southwest;
                        break;
                    case direction::northeast:
                        dir = direction::southeast;
                        break;
                    case direction::southeast:
                        dir = direction::northeast;
                        break;
                    default:
                        break;
                }
                break;
            case MIR_NS:
                switch (dir) {
                    case direction::southwest:
                        dir = direction::southeast;
                        break;
                    case direction::northwest:
                        dir = direction::northeast;
                        break;
                    case direction::northeast:
                        dir = direction::northwest;
                        break;
                    case direction::southeast:
                        dir = direction::southwest;
                        break;
                    case direction::west:
                        dir = direction::east;
                        break;
                    case direction::east:
                        dir = direction::west;
                        break;
                }
                break;
            case MIR_NESW:
                switch (dir) {
                    case direction::west:
                        dir = direction::southeast;
                        break;
                    case direction::southeast:
                        dir = direction::west;
                        break;
                    case direction::east:
                        dir = direction::northwest;
                        break;
                    case direction::northwest:
                        dir = direction::east;
                        break;
                    default:
                        break;
                }
                break;
            case MIR_NWSE:
                switch (dir) {
                    case direction::west:
                        dir = direction::northeast;
                        break;
                    case direction::northeast:
                        dir = direction::west;
                        break;
                    case direction::east:
                        dir = direction::southwest;
                        break;
                    case direction::southwest:
                        dir = direction::east;
                        break;
                    default:
                        break;
                }
                break;
            default:
                unreachable("program_walker::reflect() should only be passed a mirror");
        }
    }
};

namespace std {
template<>
struct hash<instruction_pointer> {
    inline size_t operator()(const instruction_pointer& key) const noexcept {
        return key.coords.first ^ (key.coords.second << 3) ^ (static_cast<size_t>(key.dir) << 7);
    }
};
}  // namespace std
