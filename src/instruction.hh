#pragma once

#include <cstddef>
#include "program_walker.hh"

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

// A variant-like type.
class instruction {
    using instruction_pointer = program_walker::instruction_pointer;

    template<typename T>
    using pair = std::pair<T, T>;

    // The underlying operation performed.
    enum class operation : int32_t {
        // direction-sensitive ones must be negative to ensure they never appear in source.
        BNG = -2,  // branch if negative
        JMP = -3,  // jump
        TSP = -4,  // thread spawn
        TJN = -5,  // thread join
        TKL = -6,  // thread kill
        DIRECTION_INSENSITIVE_OPS
    };

    union argument {
        nullptr_t none;
        int24_t number;
        pair<size_t> next;
        pair<pair<size_t>> choice;

        inline argument() noexcept { none = nullptr; }
    };
public:
    instruction(instruction_pointer ip, const program& program) noexcept;

    static inline instruction jump_to(pair<size_t> next) noexcept {
        argument arg;
        arg.next = next;
        return instruction(operation::JMP, arg);
    }

    static inline instruction branch_to(pair<pair<size_t>> choice) noexcept {
        argument arg;
        arg.choice = choice;
        return instruction(operation::BNG, arg);
    }

    constexpr bool is_exit() const noexcept { return m_op == operation::EXT || m_op == operation::TKL; }
    constexpr bool is_join() const noexcept { return m_op == operation::TJN; }
    constexpr bool is_nop() const noexcept { return m_op == operation::NOP; }
    std::string to_str() const noexcept;

    inline const pair<size_t>* first_if_branch() const noexcept {
        switch (m_op) {
            case operation::BNG:
            case operation::TSP:
                return &this->m_arg.choice.first;
            default:
                return nullptr;
        }
    }
private:
    inline instruction(operation op, argument arg) noexcept : m_op(op), m_arg(arg) {}

    operation m_op;
    argument m_arg;
};