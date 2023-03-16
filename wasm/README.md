# Trilangle wasm

This file outlines the file structure of this folder. For an overview of compiling Trilangle for the web, see [the main README][1].

## build_wasm.sh

This file simply invokes emscripten with the appropriate flags.

## index.*

These files are included as-is in the website -- they aren't minified in any way. GitHub compresses them to about a third of their original size, so file size isn't a great concern.

## pre.js and post.js

These files are included in the website, with minification and dead code elimination. Prefer putting code in these files over index.js, unless DCE removes it.

pre.js contains any code that modifies the Module object; anything not necessary for that goes in post.js.

[1]: ../README.md#compiling-for-the-web
