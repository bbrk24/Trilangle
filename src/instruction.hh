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
    friend class disassembler;
    friend class compiler;
    friend class assembly_scanner;

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
        struct empty {};

        empty none;
        int24_t number;
        pair<size_t> next;
        pair<pair<size_t>> choice;

        CONSTEXPR_UNION argument() noexcept { none = {}; }
    };
public:
    instruction(instruction_pointer ip, const program& program) noexcept;

    static CONSTEXPR_UNION instruction jump_to(pair<size_t> next) noexcept {
        argument arg;
        arg.next = next;
        return instruction(operation::JMP, arg);
    }

    static CONSTEXPR_UNION instruction branch_to(pair<pair<size_t>> choice) noexcept {
        argument arg;
        arg.choice = choice;
        return instruction(operation::BNG, arg);
    }

    static CONSTEXPR_UNION instruction spawn_to(pair<pair<size_t>> choice) noexcept {
        argument arg;
        arg.choice = choice;
        return instruction(operation::TSP, arg);
    }

    constexpr bool is_exit() const noexcept { return m_op == operation::EXT || m_op == operation::TKL; }
    constexpr bool is_join() const noexcept { return m_op == operation::TJN; }
    constexpr bool is_nop() const noexcept { return m_op == operation::NOP; }

    CONSTEXPR_UNION const pair<size_t>* first_if_branch() const noexcept {
        switch (m_op) {
            case operation::BNG:
            case operation::TSP:
                return &this->m_arg.choice.first;
            default:
                return nullptr;
        }
    }
protected:
    CONSTEXPR_UNION instruction(operation op, argument arg) noexcept : m_arg(arg), m_op(op) {}

    argument m_arg;
    operation m_op;
};
