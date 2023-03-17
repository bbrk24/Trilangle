/** @type {(program: string, warnings: 0 | 1, disassemble: 0 | 1, expand: 0 | 1) => (undefined | Promise<void>)} */
const wasmEntrypoint = Module.cwrap('wasm_entrypoint', null, ['string', 'number', 'number', 'number']);
/** @type {() => void} */
const wasmCancel = Module.cwrap('wasm_cancel', null);

const clearOutput = () => {
    'use strict';

    elements.output.innerHTML = '';
    elements.error.innerHTML = '';
};

/** @type {(warnings: 0 | 1, disassemble: 0 | 1) => () => void} */
const callInterpreter = (warnings, disassemble) => () => {
    'use strict';

    inputIndex = 0;
    wasmCancel();
    clearOutput();
    // By adding a 5ms delay, the previous thread has a chance to clean up after itself.
    // Without this, we may usurp its memory, and then it'll try to deallocate it in a problematic way.
    // Is this a good thing? Probably not. Does it work? Usually. Worst case, just reload the page.
    setTimeout(async () => {
        try {
            await wasmEntrypoint(elements.program.value, warnings, disassemble, 0);
        } catch (e) {
            if (!(e instanceof ExitStatus))
                elements.error.innerText += String(e);
        }
    }, 5);
};

const interpretProgram = callInterpreter(1, 0), disassembleProgram = callInterpreter(0, 1);

if (location.hash.length > 1) {
    elements.program.value = decodeURIComponent(location.hash.slice(1));
}

elements.program.oninput = () => {
    elements.urlOutBox.className = 'content-hidden';
    elements.copyButton.disabled = true;
};
