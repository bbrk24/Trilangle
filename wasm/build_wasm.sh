#!/bin/bash

set -e

common_emcc_args=(-WCL4 -Wnon-gcc -Wno-nullability-completeness
    -fno-rtti -fno-exceptions
    -sEXPORTED_RUNTIME_METHODS=ccall -sASYNCIFY
    -o worker.js
    --pre-js pre.js -sINCOMING_MODULE_JS_API='preInit,onRuntimeInitialized,noExitRuntime' --js-library library.js)

if [ "$1" = debug ]
then
    # Without the subshell, the first two waits won't actually affect the exit code of the script as a whole.
    (
        npx coffee -cMo index.js in.coffee &
        {
            npx coffee -co pre.js worker.coffee
            npx coffee -co library.js library.coffee
            emcc ../src/*.cpp "${common_emcc_args[@]}" -Wno-unused-value -Og -g2 -sASSERTIONS
        } &
        npx sass in.scss index.css --no-source-map &
        wait %1 && wait %2 && wait %3
    )
else
    (
        npx coffee -co index.js in.coffee &
        {
            npx coffee -co pre.js worker.coffee
            npx coffee -co library.js library.coffee
            emcc ../src/*.cpp "${common_emcc_args[@]}" -O3 -flto -DNDEBUG --closure 1 --closure-args='--emit_use_strict'
        } &
        npx sass in.scss index.css --no-source-map -s compressed &
        wait %1 && wait %2 && wait %3
    )
fi

rm pre.js
