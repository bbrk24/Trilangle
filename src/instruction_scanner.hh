#pragma once

#include <optional>
#include "input.hh"
#include "program_walker.hh"

class instruction_scanner : public program_walker {
public:
    CONSTEXPR_VECTOR instruction_scanner(NONNULL_PTR(const program) p) noexcept :
        program_walker(p), m_fragments(std::nullopt) {}
    instruction_scanner(const instruction_scanner&) = delete;

    virtual void write_state(std::ostream& os) = 0;
protected:
    void build_state();

    // The list of program "fragments". Each one ends with a branch, jump, thread-kill, or exit.
    std::optional<std::vector<std::optional<std::vector<instruction>>>> m_fragments;
};
