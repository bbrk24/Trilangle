#!/bin/bash

common_emcc_args=(-WCL4 -Wnon-gcc -Wno-nullability-completeness
    -fno-rtti -fno-exceptions
    -sEXPORTED_RUNTIME_METHODS=ccall
    -o out.js
    --pre-js pre.js '-sINCOMING_MODULE_JS_API=preInit,onRuntimeInitialized,noExitRuntime')

if [ "$1" = debug ]
then
    emcc ../*.cpp "${common_emcc_args[@]}" -Wno-unused-value -Og -g2 &
    npx sass in.scss index.css --no-source-map &
else
    emcc ../*.cpp "${common_emcc_args[@]}" -O3 -flto -DNDEBUG --closure 1 --closure-args='--emit_use_strict' &
    npx sass in.scss index.css --no-source-map -s compressed &
fi

npx coffee -co index.js in.coffee &

wait
