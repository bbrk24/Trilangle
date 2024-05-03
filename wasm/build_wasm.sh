#!/bin/bash

set -e

common_emcc_args=(-WCL4 -Wnon-gcc -Wimplicit-fallthrough
    -fno-rtti -fno-exceptions
    -sEXPORTED_RUNTIME_METHODS=ccall -sASYNCIFY -sASYNCIFY_IMPORTS='send_debug_info'
    --pre-js pre.js -sINCOMING_MODULE_JS_API='preInit,onRuntimeInitialized,noExitRuntime' --js-library library.js)

civet --js -c worker.civet -o pre.js &
civet --js -c library.civet -o library.js
wait

case "$1" in
    debug)
        {
            emcc ../src/*.cpp "${common_emcc_args[@]}" -Og -g2 -sASSERTIONS -o worker.js
            cp worker.wasm ldworker.wasm
            cp worker.js ldworker.js
        } &
        # emcc is so much slower than the civet and sass compilers that parallelizing them doesn't matter much
        civet --js --inline-map --comptime -c in.civet -o index.js
        civet --js --inline-map -c Colors.civet -o Colors.js
        sass in.scss:index.css lowdata.scss:lowdata.css --embed-source-map
        ;;
    lint)
        {
            emcc ../src/*.cpp "${common_emcc_args[@]}" -Werror -O0 -DNDEBUG --closure 1
            rm a.out.* || true
        } &
        civet --typecheck ./*.civet
        # Why does sass not give its version in a format it accepts?
        sass in.scss:/dev/null lowdata.scss:/dev/null --no-source-map --fatal-deprecation \
            "$(npx sass --version | cut -d' ' -f1)"
        ;;
    *)
        common_emcc_args=("${common_emcc_args[@]}" -flto -DNDEBUG --closure 1 --closure-args='--emit_use_strict')
        emcc ../src/*.cpp "${common_emcc_args[@]}" -O3 -o worker.js &
        emcc ../src/*.cpp "${common_emcc_args[@]}" -Oz -o ldworker.js &

        civet --js --comptime -c in.civet -o - | terser -c unsafe=true,unsafe_arrows=true -mo index.js --ecma 13 \
            -f wrap_func_args=false
        civet --js -c Colors.civet -o - | terser -cmf wrap_func_args=false -o Colors.js --ecma 13
        sass in.scss:index.css lowdata.scss:lowdata.css --no-source-map -s compressed

        wait -n
        ;;
esac

wait
