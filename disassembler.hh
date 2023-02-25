#pragma once

#include "program_walker.hh"
#include "input.hh"
#include <ostream>
#include <unordered_map>

// Moves a value if it's better than copying.
template<
    typename T,
    std::enable_if_t<
        sizeof (std::remove_reference_t<T>) <= sizeof (void*) && std::is_trivially_copy_constructible<T>::value,
        bool
    > = false
>
constexpr const T& maybe_move(const T& value) noexcept {
    return value;
}

// Moves a value if it's better than copying.
template<
    typename T,
    std::enable_if_t<
        std::is_nothrow_move_constructible<T>::value
            && !(
                sizeof (std::remove_reference_t<T>) <= sizeof (void*)
                && std::is_trivially_copy_constructible<T>::value
            ),
        bool
    > = true
>
constexpr std::remove_reference_t<T>&& maybe_move(T&& value) noexcept {
    return std::move(value);
}

class disassembler : public program_walker {
public:
    using program_state = std::pair<const instruction_pointer, int32_t>;

    inline disassembler(const program& p, flags f) noexcept : program_walker(p),
        m_state_ptr(nullptr),
        m_visited(),
        m_ins_num(0),
        m_flags(f) { }

    inline disassembler(disassembler&& other) noexcept : program_walker(std::move(other)),
        m_state_ptr(other.m_state_ptr),
        m_visited(maybe_move(other.m_visited)),
        m_ins_num(maybe_move(other.m_ins_num)),
        m_flags(maybe_move(other.m_flags))
    {
        other.m_state_ptr = nullptr;
    }

    explicit disassembler(const disassembler&) = default;

    inline ~disassembler() noexcept {
        if (m_state_ptr != nullptr) delete m_state_ptr;
    }

    // Write the state to the specified output stream.
    void write_state(std::ostream& os);
private:
    struct state_element {
        program_state& value;
        state_element* first_child;
        state_element* second_child;

        CONSTEXPR_DESTRUCT ~state_element() noexcept {
            if (first_child != nullptr) delete first_child;
            if (second_child != nullptr) delete second_child;
        }
    };

    void build_state();
    void build(state_element& state);

    void write(std::ostream& os, state_element& state);

    void print_op(std::ostream &os, program_state &state, bool show_nops, bool show_branch = false);

    // A binary tree of possible program states. build_state() fills it via DFS.
    state_element *m_state_ptr;
    // Track the visited program states and their locations within the disassembly.
    // The IP is used to track the state itself, and the integer is the disassembly location used for labels and jumps.
    std::unordered_map<instruction_pointer, int32_t> m_visited;
    // The number of the next instruction to be printed.
    int32_t m_ins_num;
    const flags m_flags;
};
