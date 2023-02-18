#pragma once

#include "program_walker.hh"
#include <ostream>
#include <vector>
#include <unordered_map>

class disassembler : public program_walker {
public:
    using program_state = std::pair<const instruction_pointer, long>;

    inline disassembler(const program& p) noexcept : program_walker(p),
        m_state_ptr(nullptr),
        m_visited(),
        m_ins_num(0L) { }

    disassembler(const disassembler&) = delete;

    inline ~disassembler() noexcept {
        if (m_state_ptr != nullptr) delete m_state_ptr;
    }

    void write_state(std::wostream& os);
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

    void write(std::wostream& os, state_element& state);

    state_element* m_state_ptr;
    std::unordered_map<instruction_pointer, long, instruction_pointer::hash> m_visited;
    long m_ins_num;
};
