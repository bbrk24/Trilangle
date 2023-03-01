#!/bin/sh
emcc ../*.cpp \
    -WCL4 -Wnon-gcc \
    -O3 -flto -fno-rtti -fno-exceptions -DNDEBUG \
    -sEXPORTED_RUNTIME_METHODS=ccall -sASYNCIFY \
    -o out.js \
    --pre-js pre.js -sINCOMING_MODULE_JS_API=preInit,noExitRuntime
