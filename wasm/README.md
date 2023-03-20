# Trilangle wasm

This file outlines the file structure of this folder. For an overview of compiling Trilangle for the web, see [the main README][1].

## build_wasm.sh

This file simply invokes emscripten and sass with the appropriate flags.

## index.*

These files are included as-is in the website -- they aren't minified in any way. GitHub compresses them to about a third of their original size, so file size isn't a great concern.

## input.scss

This file is transformed into out.css. Prefer putting styling information here when possible.

## pre.js

This file is used in a worker. It deals with the webassembly code directly. It is minified and optimized before its inclusion in the website.

[1]: ../README.md#compiling-for-the-web
