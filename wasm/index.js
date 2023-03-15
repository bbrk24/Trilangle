'use strict';
// For some reason, these functions are considered dead code if I put them in post.js, but the other ones aren't.
function expandInput() {
    clearOutput();
    Module.ccall('wasm_entrypoint', null, ['string', 'number', 'number', 'number'], [elements.program.value, 0, 0, 1]);
    elements.program.value = elements.output.innerText;
    elements.output.innerHTML = '';
}
function contractInput() {
    clearOutput();
    // Remove spaces and newlines (intentionally not other whitespace)
    /** @type {string} */
    let programText = elements.program.value.replace(/ |\n/g, '');
    // programText.length is wrong when there's high Unicode characters
    const programLength = [...programText].length;
    // Calculate the largest triangular number less than the length. minLength is 1 more than that
    const temp = Math.ceil(Math.sqrt(2 * programLength) - 1.5);
    const minLength = 1 + temp * (temp + 1) / 2;
    // Remove trailing dots, but not too many
    if (programLength !== minLength) {
        programText = programText.replace(new RegExp(`\\.{0,${programLength - minLength}}\$`), '');
    }
    elements.program.value = programText;
}
