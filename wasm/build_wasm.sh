#!/bin/sh
emcc ../*.cpp \
    -WCL4 -Wnon-gcc -Wno-nullability-completeness \
    -O3 -flto -fno-rtti -fno-exceptions -DNDEBUG \
    -sEXPORTED_RUNTIME_METHODS=ccall -sASYNCIFY \
    -o out.js \
    --pre-js pre.js --post-js post.js -sINCOMING_MODULE_JS_API=preInit,noExitRuntime
