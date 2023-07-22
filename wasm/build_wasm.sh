#!/bin/bash

set -e

common_emcc_args=(-WCL4 -Wnon-gcc -Wno-nullability-completeness
    -fno-rtti -fno-exceptions
    -sEXPORTED_RUNTIME_METHODS=ccall -sASYNCIFY -sASYNCIFY_IMPORTS='send_debug_info'
    -o worker.js
    --pre-js pre.js -sINCOMING_MODULE_JS_API='preInit,onRuntimeInitialized,noExitRuntime' --js-library library.js)

npx coffee -co pre.js worker.coffee
npx coffee -co library.js library.coffee

if [ "$1" = debug ]
then
    emcc ../src/*.cpp "${common_emcc_args[@]}" -Wno-unused-value -Og -g2 -sASSERTIONS &
    # emcc is so much slower than the coffeescript and sass compilers that parallelizing them doesn't matter much
    npx coffee -cMo index.js in.coffee
    npx sass in.scss index.css --embed-source-map
else
    emcc ../src/*.cpp "${common_emcc_args[@]}" -O3 -flto -DNDEBUG --closure 1 --closure-args='--emit_use_strict' &
    npx coffee -co index.js in.coffee
    npx sass in.scss index.css --no-source-map -s compressed
fi

wait
