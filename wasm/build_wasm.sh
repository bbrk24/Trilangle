#!/bin/bash

set -e

common_emcc_args=(-WCL4 -Wnon-gcc -Wimplicit-fallthrough
    -fno-rtti -fno-exceptions
    -sEXPORTED_RUNTIME_METHODS=ccall -sASYNCIFY -sASYNCIFY_IMPORTS='send_debug_info'
    -o worker.js
    --pre-js pre.js -sINCOMING_MODULE_JS_API='preInit,onRuntimeInitialized,noExitRuntime' --js-library library.js)

npx coffee -o pre.js worker.coffee &
npx coffee -o library.js library.coffee
wait

case "$1" in
    debug)
        emcc ../src/*.cpp "${common_emcc_args[@]}" -Og -g2 -sASSERTIONS &
        # emcc is so much slower than the coffeescript and sass compilers that parallelizing them doesn't matter much
        npx coffee -Mo index.js in.coffee
        npx sass in.scss index.css --embed-source-map
        ;;
    lint)
        emcc ../src/*.cpp "${common_emcc_args[@]}" -Werror -O0 -DNDEBUG --closure 1 &
        npx coffee -p in.coffee >/dev/null
        # Why does sass not give its version in a format it accepts?
        npx sass in.scss /dev/null --no-source-map --fatal-deprecation "$(npx sass --version | cut -d' ' -f1)"
        ;;
    *)
        emcc ../src/*.cpp "${common_emcc_args[@]}" -O3 -flto -DNDEBUG --closure 1 --closure-args='--emit_use_strict' &
        npx coffee -o index.js in.coffee
        npx sass in.scss index.css --no-source-map -s compressed
        ;;
esac

wait
