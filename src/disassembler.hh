#pragma once

#include <ostream>
#include "input.hh"
#include "instruction.hh"

class disassembler : public program_walker {
public:
    inline disassembler(NONNULL_PTR(const program) p, flags f) : program_walker(p), m_fragments(nullptr), m_flags(f) {}

    disassembler(const disassembler&) = delete;
    ~disassembler() noexcept;

    // Write the state to the specified output stream.
    void write_state(std::ostream& os);
private:
    void build_state();

    // The list of program "fragments". Each one ends with a branch, jump, threadkill, or exit.
    std::vector<std::vector<instruction>*>* m_fragments;

    const flags m_flags;
};
