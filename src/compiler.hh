#pragma once

#include "instruction_scanner.hh"

class compiler : public instruction_scanner {
public:
    CONSTEXPR_VECTOR compiler(NONNULL_PTR(const program) p) : instruction_scanner(p) {}
    void write_state(std::ostream& os);
private:
    static void get_c_code(const instruction& i, std::ostream& os);
};
