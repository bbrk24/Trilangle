#pragma once

#include "program.hh"

enum class direction : char {
    // Bitfield representation: { north, !(north || south), east }. This is designed so the default direction
    // (southwest) is 0 and using this bitfield rather than consecutive values reduces binary size slightly.

    southwest = 0b000,
    west = 0b010,
    northwest = 0b100,
    northeast = 0b101,
    east = 0b011,
    southeast = 0b001,
    invalid = 0b110,
};

#define UNREACHABLE_INVALID_DIR \
    case direction::invalid: \
        unreachable("invalid direction must never be passed")

class program_walker {
public:
    constexpr program_walker(NONNULL_PTR(const program) p) noexcept : m_program(p) {}

    // Pack this struct, so that the sizeof (size_t) - 1 bytes leftover at the end can be used by other variables in a
    // larger object (i.e. disassembler and interpreter). I know pragmas aren't generally portable, but pack(...) is
    // supported by MSVC, clang, and gcc.
#pragma pack(push, 1)
    struct instruction_pointer {
        std::pair<size_t, size_t> coords;
        direction dir;

        constexpr bool operator==(const instruction_pointer& rhs) const noexcept {
            return this->coords == rhs.coords && this->dir == rhs.dir;
        }
    };
#pragma pack(pop)

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
                UNREACHABLE_INVALID_DIR;
        }
    }
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
                    case direction::east:
                        FALLTHROUGH
                    case direction::west:
                        break;
                        UNREACHABLE_INVALID_DIR;
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
                        UNREACHABLE_INVALID_DIR;
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
                    case direction::northeast:
                        FALLTHROUGH
                    case direction::southwest:
                        break;
                        UNREACHABLE_INVALID_DIR;
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
                    case direction::northwest:
                        FALLTHROUGH
                    case direction::southeast:
                        break;
                        UNREACHABLE_INVALID_DIR;
                }
                break;
            default:
                unreachable("program_walker::reflect() should only be passed a mirror");
        }
    }

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4702 \
)  // Unreachable code, which it complains about when constant-folding the template instantiation
#endif

    // Move the IP according to the branch instruction.
    template<typename T>
    static void branch(direction& dir, int24_t bng, T go_left) noexcept(noexcept(go_left())) {
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
                        FALLTHROUGH
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
                        FALLTHROUGH
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
                        UNREACHABLE_INVALID_DIR;
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
                        FALLTHROUGH
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
                        FALLTHROUGH
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
                        UNREACHABLE_INVALID_DIR;
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
                        FALLTHROUGH
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
                        UNREACHABLE_INVALID_DIR;
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
                        FALLTHROUGH
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
                        UNREACHABLE_INVALID_DIR;
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
                        FALLTHROUGH
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
                        UNREACHABLE_INVALID_DIR;
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
                        FALLTHROUGH
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
                        UNREACHABLE_INVALID_DIR;
                }
                break;
            default:
                unreachable("program_walker::branch() should only be passed a branch");
        }
    }

#ifdef _MSC_VER
#pragma warning(pop)
#endif
};

namespace std {
template<>
struct hash<program_walker::instruction_pointer> {
    inline size_t operator()(const program_walker::instruction_pointer& key) const noexcept {
        return key.coords.first ^ (key.coords.second << 3) ^ (static_cast<size_t>(key.dir) << 7);
    }
};
}  // namespace std
