#pragma once

#include "disassembler.hh"

class compiler : public disassembler {
public:
    CONSTEXPR_VECTOR compiler(NONNULL_PTR(const program) p, flags f) : disassembler(p, f) {}
    void write_state(std::ostream& os) override;
private:
    static void get_c_code(const instruction& i, std::ostream& os);
};
