#include <clocale>
#include <iostream>
#include "disassembler.hh"
#include "interpreter.hh"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE MAYBE_UNUSED
#endif

inline void execute(const std::string& prg, flags f) {
    // The only thing cstdio and iostream need to be synced for is the ferror check when pipekill is set.
    std::ios::sync_with_stdio(f.pipekill);

    program p(prg);

    if (f.disassemble) {
        disassembler d(&p, f);
        d.write_state(std::cout);
    } else if (f.expand) {
        std::cout << p << std::flush;
    } else {
        interpreter i(&p, f);
        i.run();
    }
}

#ifndef __EMSCRIPTEN__
int main(int argc, const char** argv) {
    // Set locale so that getwchar works as expected
    setlocale(LC_ALL, "");

    flags f;
    std::string program_text = parse_args(argc, argv, f);
    execute(program_text, f);
}
#endif

extern "C" {
EMSCRIPTEN_KEEPALIVE void wasm_entrypoint(const char* program_text, int warnings, int disassemble, int expand) {
    // Reset EOF from previous runs
    clearerr(stdin);
    // Input and output don't need to be synced on the web
    std::cin.tie(nullptr);

    flags f;
    f.warnings = warnings;
    f.disassemble = disassemble;
    f.hide_nops = disassemble;
    f.expand = expand;

    execute(program_text, f);
}
}
