#pragma once

#include "program_walker.hh"
#include <ostream>
#include <vector>
#include <unordered_set>

class disassembler : public program_walker {
public:
    inline disassembler(const program& p) noexcept : program_walker(p),
        m_state_ptr(nullptr),
        m_visited(),
        m_lbl_num(0) { }

    disassembler(const disassembler&) = delete;

    inline ~disassembler() noexcept {
        if (m_state_ptr != nullptr) delete m_state_ptr;
    }

    void write_state(std::wostream& os);
private:
    struct program_state {
        instruction_pointer ip;

        constexpr bool operator==(const program_state& rhs) const noexcept {
            return this->ip.coords == rhs.ip.coords && this->ip.dir == rhs.ip.dir;
        }

        struct hash {
            inline size_t operator()(const program_state& key) const noexcept {
                size_t first_hash = key.ip.coords.first;
                size_t second_hash = (key.ip.coords.second << (4 * sizeof (size_t))) | (key.ip.coords.second >> (4 * sizeof (size_t)));
                size_t direction_hash = std::hash<direction>()(key.ip.dir);

                return first_hash ^ second_hash ^ direction_hash;
            }
        };
    };

    struct state_element {
        const program_state& value;
        state_element* first_child;
        state_element* sibling;

        CONSTEXPR_DESTRUCT ~state_element() noexcept {
            if (sibling != nullptr) delete sibling;
            if (first_child != nullptr) delete first_child;
        }
    };

    void build_state();
    void build(state_element& state);

    void write(std::wostream& os, const state_element& state, bool label);

    state_element* m_state_ptr;
    std::unordered_set<program_state, program_state::hash> m_visited;
    int m_lbl_num;
};
