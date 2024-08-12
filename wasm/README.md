# Trilangle wasm

This file outlines the file structure of this folder. For an overview of compiling Trilangle for the web, see [the main README][1].

## build_wasm.sh

This file simply invokes emscripten and sass with the appropriate flags. If invoked with the argument "debug", it doesn't perform optimizations, which should result in faster compile times. If invoked with the argument "lint", it only verifies that the builds will succeed and does not produce all of the output files.

## lint.sh

This file performs various linting passes, primarily using eslint and stylelint.

## index.html

This file is included as-is in the website.

## in.civet and in.scss

These files are transpiled to index.js and index.css respectively.

## Colors.civet

This file contains a helper class for managing the highlight colors of threads in the visual debugger.

## library.civet

This file includes functions that bridge between C++ and JS. The functions are declared as `extern "C"` in the C++ code, but actually defined in this file.

## lowdata.scss and lowdata.html

These are stripped-down versions with all the same functionality but which look more barebones and may be less performant.

## worker.civet

This file is used in a worker. It deals with the webassembly code directly. The output JavaScript is minified and optimized before its inclusion in the website.

[1]: ../README.md#compiling-for-the-web
