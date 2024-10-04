#define _CRT_SECURE_NO_WARNINGS 1

#include "assembly_scanner.hh"
#include "compiler.hh"
#include "disassembler.hh"
#include "interpreter.hh"

[[noreturn]] inline void empty_program() {
    std::cerr << "What program do you want me to run? C'mon, give me something to work with." << std::endl;
    exit(EX_DATAERR);
}

inline void execute(const std::string& prg, flags f) {
#ifdef NO_BUFFER
    setvbuf(stdout, nullptr, _IONBF, 0);
#else
    // The only thing cstdio and iostream need to be synced for is the ferror check when pipekill is set.
    std::ios::sync_with_stdio(f.pipekill);
#endif

    if (f.assembly) {
        assembly_scanner as(prg);
        if (as.get_fragments().size() == 0) {
            empty_program();
        }
        interpreter i(as, f);
        i.run();
        return;
    }

    program p(prg);
    if (p.side_length() == 0) {
        empty_program();
    }

    if (f.disassemble) {
        disassembler d(&p, f);
        d.write_state(std::cout);
    } else if (f.expand) {
        std::cout << p << std::flush;
    } else if (f.compile) {
        compiler c(&p, f);
        c.write_state(std::cout);
    } else {
        program_walker pw(&p);
        interpreter i(pw, f);
        i.run();
    }
}

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>

extern "C" EMSCRIPTEN_KEEPALIVE void
wasm_entrypoint(CONST_C_STR program_text, int disassemble, int expand, int debug, int assembly) {
    // Reset EOF from previous runs
    clearerr(stdin);
    // Input and output don't need to be synced on the web
    std::cin.tie(nullptr);

    flags f;
    f.warnings = true;
    f.disassemble = disassemble;
    // Disabled so that the output is compatible with -A
    // f.hide_nops = disassemble;
    f.expand = expand;
    f.debug = debug;
    f.show_stack = true;
    f.assembly = assembly;

    execute(program_text, f);
}
#else
int main(int argc, _In_reads_z_(argc) const char** argv) {
    flags f;
    std::string program_text = parse_args(argc, argv, f);
    execute(program_text, f);
}
#endif
