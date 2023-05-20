#pragma once

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

    // The underlying operation performed.
    enum class operation : char {
        BNG,  // branch if negative
        JMP,  // jump
        TSP,  // thread spawn
        TJN,  // thread join
        TKL,  // thread kill
        DIRECTION_INSENSITIVE_OPS
    };

    union argument {
        nullptr_t none;
        int24_t number;
        std::pair<size_t, size_t> next;
        std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> choice;

        inline argument() noexcept { none = nullptr; }
    };
public:
    instruction(instruction_pointer ip, const program& program) noexcept;

    static inline instruction jump_to(std::pair<size_t, size_t> next) noexcept {
        argument arg;
        arg.next = next;
        return instruction(operation::JMP, arg);
    }

    static inline instruction branch_to(std::pair<std::pair<size_t, size_t>, std::pair<size_t, size_t>> choice
    ) noexcept {
        argument arg;
        arg.choice = choice;
        return instruction(operation::BNG, arg);
    }

    constexpr bool is_exit() const noexcept { return m_op == operation::EXT || m_op == operation::TKL; }
    constexpr bool is_join() const noexcept { return m_op == operation::TJN; }
    constexpr bool is_nop() const noexcept { return m_op == operation::NOP; }
    std::string to_str() const noexcept;

    const std::pair<size_t, size_t>* first_if_branch() const noexcept {
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
