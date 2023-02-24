#include "interpreter.hh"
#include "disassembler.hh"
#include <iostream>
#include <clocale>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE MAYBE_UNUSED
#endif

inline void execute(std::string prg, flags f) {
    program p(prg);

    if (f.disassemble) {
        disassembler d(p, f);
        d.write_state(std::wcout);
    } else {
        interpreter i(p, f);
        i.run();
    }
}

int main(int argc, const char** argv) {
#ifndef __EMSCRIPTEN__
    flags f;
    std::string program_text = parse_args(argc, argv, f);
    execute(program_text, f);
#endif
}

extern "C" {
    EMSCRIPTEN_KEEPALIVE void wasm_entrypoint(
        const char* program_text,
        int warnings,
        int disassemble,
        int hide_nops
    ) {
        // Set locale so that putwchar works as expected
        setlocale(LC_ALL, "");
        // Reset EOF from previous runs
        clearerr(stdin);

        flags f;
        f.warnings = warnings;
        f.disassemble = disassemble;
        f.hide_nops = hide_nops;

        std::string prg(program_text);
        execute(prg, f);
    }

    EMSCRIPTEN_KEEPALIVE void wasm_cancel() {
        interpreter::stop_all();
    }
}
