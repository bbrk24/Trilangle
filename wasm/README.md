# Trilangle wasm

This file outlines the file structure of this folder. For an overview of compiling Trilangle for the web, see [the main README][1].

## build_wasm.sh

This file simply invokes emscripten and sass with the appropriate flags. If invoked with the argument "debug", it doesn't perform optimizations, which should result in faster compile times.

## index.html

This file is included as-is in the website.

## in.coffee and in.scss

These files are transpiled to index.js and index.css respectively.

## pre.js

This file is used in a worker. It deals with the webassembly code directly. It is minified and optimized before its inclusion in the website.

[1]: ../README.md#compiling-for-the-web
