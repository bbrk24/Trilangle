#pragma once

#include "program.hh"

enum class direction : char {
    southwest, west, northwest, northeast, east, southeast
};

class program_walker {
public:
    constexpr program_walker(const program& p) noexcept : m_program(p) { }

    struct instruction_pointer {
        std::pair<size_t, size_t> coords;
        direction dir;

        constexpr bool operator==(const instruction_pointer& rhs) const noexcept {
            return this->coords == rhs.coords && this->dir == rhs.dir;
        }

        struct hash {
            inline size_t operator()(const instruction_pointer& key) const noexcept {
                size_t first_hash = key.coords.first;
                size_t second_hash = (key.coords.second << (4 * sizeof(size_t))) | (key.coords.second >> (4 * sizeof(size_t)));
                size_t direction_hash = std::hash<direction>()(key.dir);

                return first_hash ^ second_hash ^ direction_hash;
            }
        };
    };

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
protected:
    const program& m_program;

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
                    case direction::east: FALLTHROUGH
                    case direction::west:
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
                    case direction::northeast: FALLTHROUGH
                    case direction::southwest:
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
                    case direction::northwest: FALLTHROUGH
                    case direction::southeast:
                        break;
                }
                break;
            default:
                unreachable();
        }
    }

    // Move the IP according to the branch instruction.
    template<typename T>
    static void branch(direction& dir, int24_t bng, T go_left) noexcept(noexcept(go_left())) {
        switch (bng) {
            case BNG_E:
                switch (dir) {
                    case direction::west:
                        if (go_left()) {
                            dir = direction::southwest;
                        } else {
                            dir = direction::northwest;
                        }
                        break;
                    case direction::northeast: FALLTHROUGH
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
            case BNG_W:
                switch (dir) {
                    case direction::east:
                        if (go_left()) {
                            dir = direction::northeast;
                        } else {
                            dir = direction::southeast;
                        }
                        break;
                    case direction::northwest: FALLTHROUGH
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
                    case direction::northwest: FALLTHROUGH
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
                    case direction::southeast: FALLTHROUGH
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
                    case direction::northeast: FALLTHROUGH
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
                    case direction::southwest: FALLTHROUGH
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
                unreachable();
        }
    }
};
