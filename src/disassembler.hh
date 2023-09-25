#pragma once

#include <iosfwd>
#include "input.hh"
#include "instruction.hh"

class disassembler : public program_walker {
public:
    CONSTEXPR_VECTOR disassembler(NONNULL_PTR(const program) p, flags f) :
        program_walker(p), m_fragments(nullptr), m_flags(f) {}

    disassembler(const disassembler&) = delete;

    CONSTEXPR_VECTOR ~disassembler() noexcept {
        if (m_fragments == nullptr) {
            return;
        }
        for (std::vector<instruction>* frag : *m_fragments) {
            delete frag;
        }
        delete m_fragments;
    }

    // Write the state to the specified output stream.
    virtual void write_state(std::ostream& os);
protected:
    void build_state();

    // The list of program "fragments". Each one ends with a branch, jump, thread-kill, or exit.
    std::vector<std::vector<instruction>*>* m_fragments;
private:
    const flags m_flags;
};
