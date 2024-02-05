#!/bin/bash

set -e

common_emcc_args=(-WCL4 -Wnon-gcc -Wimplicit-fallthrough
    -fno-rtti -fno-exceptions
    -sEXPORTED_RUNTIME_METHODS=ccall -sASYNCIFY -sASYNCIFY_IMPORTS='send_debug_info'
    --pre-js pre.js -sINCOMING_MODULE_JS_API='preInit,onRuntimeInitialized,noExitRuntime' --js-library library.js)

npx civet --js -c worker.civet -o pre.js &
npx civet --js -c library.civet -o library.js
wait

case "$1" in
    debug)
        {
            emcc ../src/*.cpp "${common_emcc_args[@]}" -Og -g2 -sASSERTIONS -o worker.js
            cp worker.wasm ldworker.wasm
            cp worker.js ldworker.js
        } &
        # emcc is so much slower than the civet and sass compilers that parallelizing them doesn't matter much
        npx civet --js --inline-map -c in.civet -o index.js
        npx civet --js --inline-map -c Colors.civet -o Colors.js
        npx sass in.scss:index.css lowdata.scss:lowdata.css --embed-source-map
        ;;
    lint)
        {
            emcc ../src/*.cpp "${common_emcc_args[@]}" -Werror -O0 -DNDEBUG --closure 1
            rm a.out.* || true
        } &
        npx civet <Colors.civet >/dev/null
        npx civet <in.civet >/dev/null
        # Why does sass not give its version in a format it accepts?
        npx sass in.scss:/dev/null lowdata.scss:/dev/null --no-source-map --fatal-deprecation \
            "$(npx sass --version | cut -d' ' -f1)"
        ;;
    *)
        common_emcc_args=("${common_emcc_args[@]}" -flto -DNDEBUG --closure 1 --closure-args='--emit_use_strict')
        {
            emcc ../src/*.cpp "${common_emcc_args[@]}" -O3 -o worker.js &
            emcc ../src/*.cpp "${common_emcc_args[@]}" -Oz -o ldworker.js
            wait
        } &
        npx civet --js -c in.civet -o - | npx terser -c keep_fargs=false,unsafe=true,unsafe_arrows=true -mo index.js \
            --ecma 12 -f wrap_func_args=false
        npx civet --js -c Colors.civet -o - | npx terser -mc keep_fargs=false,keep_classnames=true,unsafe_arrows=true \
            --ecma 12 -o Colors.js -f wrap_func_args=false
        npx sass in.scss:index.css lowdata.scss:lowdata.css --no-source-map -s compressed
        ;;
esac

wait
