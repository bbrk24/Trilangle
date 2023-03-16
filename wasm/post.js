const clearOutput = () => {
    'use strict';

    elements.output.innerHTML = '';
    elements.error.innerHTML = '';
};

/** @type {(warnings: 0 | 1, disassemble: 0 | 1) => () => void} */
const callInterpreter = (warnings, disassemble) => () => {
    'use strict';

    inputIndex = 0;
    Module.ccall('wasm_cancel', null);
    clearOutput();
    // By adding a 5ms delay, the previous thread has a chance to clean up after itself.
    // Without this, we may usurp its memory, and then it'll try to deallocate it in a problematic way.
    // Is this a good thing? Probably not. Does it work? Usually. Worst case, just reload the page.
    setTimeout(async () => {
        try {
            await Module.ccall(
                'wasm_entrypoint',
                null,
                ['string', 'number', 'number', 'number'],
                [elements.program.value, warnings, disassemble, 0]
            );
        } catch (e) {
            if (!(e instanceof ExitStatus))
                elements.error.innerText += String(e);
        }
    }, 5);
};

const interpretProgram = callInterpreter(1, 0), disassembleProgram = callInterpreter(0, 1);

(() => {
    'use strict';
    // Code here runs immediately. I'm using an IIFE here so that any variables declared don't pollute the global
    // object.

    const p = new URL(location).searchParams.get('p');
    if (p !== null) {
        elements.program.value = p;
    }

    elements.program.oninput = () => elements.urlOutBox.className = 'content-hidden';
})();
