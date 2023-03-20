#!/bin/sh
emcc ../*.cpp \
    -WCL4 -Wnon-gcc -Wno-nullability-completeness \
    -O3 -flto -fno-rtti -fno-exceptions -DNDEBUG --closure 1 \
    --closure-args='--emit_use_strict' -sEXPORTED_RUNTIME_METHODS=ccall \
    -o out.js \
    --pre-js pre.js -sINCOMING_MODULE_JS_API=preInit,onRuntimeInitialized,noExitRuntime &
npx sass input.scss out.css --no-source-map &
wait
