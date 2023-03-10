#pragma once

#include <string>
#include "int24.hh"

struct flags {
    bool debug : 1;
    bool show_stack : 1;
    bool warnings : 1;
    bool pipekill : 1;
    bool disassemble : 1;
    bool hide_nops : 1;
    bool expand : 1;

    constexpr flags() noexcept :
        debug(false),
        show_stack(false),
        warnings(false),
        pipekill(false),
        disassemble(false),
        hide_nops(false),
        expand(false) {}

    constexpr bool is_valid() const noexcept {
        return !(
            (show_stack && !debug) || ((debug || warnings || pipekill) && disassemble) || (hide_nops && !disassemble)
            || (expand && (debug || warnings || pipekill || disassemble))
        );
    }
};

// Read input file or STDIN, and return its contents. Parse other flags as appropriate.
MAYBE_UNUSED std::string parse_args(int argc, const char** argv, flags& f);

// Gets a single unicode character from STDIN. Returns -1 for EOF.
int24_t getunichar() noexcept;
