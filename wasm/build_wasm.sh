#!/bin/sh
emcc ../*.cpp \
    -O2 -flto -fno-rtti -fno-exceptions \
    -sEXPORTED_RUNTIME_METHODS=ccall \
    -o out.js \
    --pre-js pre.js -sINCOMING_MODULE_JS_API=preInit,noExitRuntime
