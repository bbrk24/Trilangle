#pragma once

#include "instruction_scanner.hh"

class disassembler : public instruction_scanner {
public:
    CONSTEXPR_VECTOR disassembler(NONNULL_PTR(const program) p, flags f) noexcept :
        instruction_scanner(p), m_flags(f) {}

    // Write the state to the specified output stream.
    void write_state(std::ostream& os);
private:
    const flags m_flags;
    static std::string to_str(const instruction& i) noexcept;
};
