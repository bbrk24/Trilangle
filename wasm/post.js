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

    elements.disassembleButton.disabled = true;
    elements.expandButton.disabled = true;

    // By adding a 5ms delay, the previous thread has a chance to clean up after itself.
    // Without this, we may usurp its memory, and then it'll try to deallocate it in a problematic way.
    // Is this a good thing? Probably not. Does it work? Usually. Worst case, just reload the page.
    setTimeout(async () => {
        elements.runStopButton.textContent = 'Stop';
        try {
            const result = wasmEntrypoint(elements.program.value, warnings, disassemble, 0);
            if (typeof result == 'object' && 'then' in result)
                await result;
        } catch (e) {
            if (!(e instanceof ExitStatus))
                elements.error.innerText += String(e);
        }
        elements.expandButton.disabled = false;
        elements.disassembleButton.disabled = false;
        elements.runStopButton.textContent = 'Run!';
    }, 5);
};

const interpretProgram = callInterpreter(1, 0), disassembleProgram = callInterpreter(0, 1);

elements.program.oninput = () => {
    'use strict';
    elements.urlOutBox.className = 'content-hidden';
    elements.urlButton.textContent = 'Generate URL';
};

// Use an IIFE to prevent polluting the global object
(() => {
    'use strict';

    // Set the button width
    const width = elements.runStopButton.offsetWidth;
    elements.runStopButton.textContent = 'Run!';
    setTimeout(
        () => elements.runStopButton.style.width = `${0.5 + Math.max(width, elements.runStopButton.offsetWidth)}px`
    );

    // Get the program from the URL, if present
    if (location.hash.length > 1) {
        elements.program.value = decodeURIComponent(location.hash.slice(1));
    }

    // Set the grid columns and overflow behavior
    const styleEl = document.createElement('style');
    document.head.appendChild(styleEl);
    const emSize = parseFloat(getComputedStyle(document.body).fontSize);
    onresize = () => {
        if (document.querySelector('main').clientWidth / 2 > elements.program.offsetWidth + emSize)
            styleEl.innerHTML = `.grid { grid-template-columns: 1fr 1fr; max-height: 100vh; }
.out-container {
    max-height: calc(100vh - 0.5em - ${
                1 + elements.programFieldset.offsetHeight + elements.clearContainer.offsetHeight +
                elements.outputLabel.offsetHeight + elements.footer.offsetHeight}px);
    padding-bottom: 1em;
}`;
        else
            styleEl.innerHTML = '.grid { grid-template-columns: 1fr; }';
    };
    new ResizeObserver(onresize).observe(elements.programFieldset);
    onresize();

    // Restrict the URL width
    elements.urlOut.style.width = `${elements.program.clientWidth}px`;
    elements.urlOutBox.style.width = `${elements.program.clientWidth}px`;
})();
