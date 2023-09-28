#pragma once

#include "instruction_scanner.hh"

class compiler : public instruction_scanner {
public:
    CONSTEXPR_VECTOR compiler(NONNULL_PTR(const program) p, flags f) : instruction_scanner(p), m_flags(f) {}
    void write_state(std::ostream& os);
private:
    flags m_flags;
    static void get_c_code(const instruction& i, std::ostream& os, bool assume_ascii);
};
