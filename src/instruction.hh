#pragma once

#include "program.hh"

struct instruction_pointer;

// A variant-like type.
class instruction {
    friend class assembly_scanner;
    friend class program_walker;

    template<typename T>
    using pair = std::pair<T, T>;
public:
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

    CONSTEXPR_UNION const pair<size_t>* second_if_branch() const noexcept {
        switch (m_op) {
            case operation::BNG:
            case operation::TSP:
                return &this->m_arg.choice.second;
            default:
                return nullptr;
        }
    }

    constexpr operation get_op() const noexcept { return m_op; }
    CONSTEXPR_UNION const argument& get_arg() const noexcept { return m_arg; }
protected:
    CONSTEXPR_UNION instruction(operation op, argument arg) noexcept : m_arg(arg), m_op(op) {}

    argument m_arg;
    operation m_op;
};
