#pragma once

#include <ostream>
#include <unordered_map>
#include "input.hh"
#include "program_walker.hh"

class disassembler : public program_walker {
public:
    using program_state = std::pair<const instruction_pointer, int32_t>;

    inline disassembler(NONNULL_PTR(const program) p, flags f) :
        program_walker(p), m_state_ptr(nullptr), m_visited(), m_ins_num(0), m_flags(f) {}

    disassembler(const disassembler&) = delete;
    ~disassembler() noexcept;

    // Write the state to the specified output stream.
    void write_state(std::ostream& os);
private:
    struct state_element {
        program_state& value;
        state_element* first_child;
        state_element* second_child;

        CONSTEXPR_DESTRUCT ~state_element() noexcept {
            if (first_child != nullptr) {
                delete first_child;
            }
            if (second_child != nullptr) {
                delete second_child;
            }
        }
    };

    void build_state();
    void build(state_element& state);

    void write(std::ostream& os, const state_element& state);

    void print_op(std::ostream& os, program_state& state, bool show_nops, direction from_dir, bool show_branch = false);

    // A binary tree of possible program states. build_state() fills it via DFS.
    state_element* m_state_ptr;
    // Track the visited program states and their locations within the disassembly.
    // The IP is used to track the state itself, and the integer is the disassembly location used for labels and jumps.
    std::unordered_map<instruction_pointer, int32_t> m_visited;
    // The number of the next instruction to be printed.
    int32_t m_ins_num;
    const flags m_flags;
};
