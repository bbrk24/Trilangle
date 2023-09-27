#include <iostream>
#include "compiler.hh"
#include "disassembler.hh"
#include "interpreter.hh"

inline void execute(const std::string& prg, flags f) {
    // The only thing cstdio and iostream need to be synced for is the ferror check when pipekill is set.
    std::ios::sync_with_stdio(f.pipekill);

    program p(prg);

    if (f.disassemble) {
        disassembler d(&p, f);
        d.write_state(std::cout);
    } else if (f.expand) {
        std::cout << p << std::flush;
    } else if (f.compile) {
        compiler c(&p);
        c.write_state(std::cout);
    } else {
        interpreter i(&p, f);
        i.run();
    }
}

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>

extern "C" EMSCRIPTEN_KEEPALIVE void wasm_entrypoint(const char* program_text, int disassemble, int expand, int debug) {
    // Reset EOF from previous runs
    clearerr(stdin);
    // Input and output don't need to be synced on the web
    std::cin.tie(nullptr);

    flags f;
    f.warnings = true;
    f.disassemble = disassemble;
    f.hide_nops = disassemble;
    f.expand = expand;
    f.debug = debug;
    f.show_stack = true;

    execute(program_text, f);
}
#else
int main(int argc, const char** argv) {
    flags f;
    std::string program_text = parse_args(argc, argv, f);
    execute(program_text, f);
}
#endif
