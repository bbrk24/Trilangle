/** @type {(program: string, warnings: 0 | 1, disassemble: 0 | 1, expand: 0 | 1) => (undefined | Promise<void>)} */
const wasmEntrypoint = Module.cwrap('wasm_entrypoint', null, ['string', 'number', 'number', 'number']);
/** @type {(...args: never[]) => void} */
const wasmCancel = Module.cwrap('wasm_cancel', null);

const clearOutput = () => {
    'use strict';

    elements.stdout.innerHTML = '';
    elements.stderr.innerHTML = '';
}, generateContracted = () => {
    'use strict';
    // Remove an unnecessary shebang
    let programText = elements.program.value.replace(/^#![^\n]*\n/, '');
    // Remove spaces and newlines (intentionally not other whitespace)
    programText = programText.replace(/ |\n/g, '');
    // programText.length is wrong when there's high Unicode characters
    const programLength = [...programText].length;
    // Calculate the largest triangular number less than the length. minLength is 1 more than that
    const temp = Math.ceil(Math.sqrt(2 * programLength) - 1.5);
    const minLength = 1 + temp * (temp + 1) / 2;
    // Remove trailing dots, but not too many
    if (programLength !== minLength)
        programText = programText.replace(new RegExp(`\\.{0,${programLength - minLength}}\$`), '');
    // Fix an accidentally-created shebang
    programText = programText.replace(/^#!/, '#\n!');
    return programText;
}, generateURL = () => {
    'use strict';
    const newURL = `${location.href.split('#')[0]}#${encodeURIComponent(generateContracted())}`;
    history.pushState({}, '', newURL);
    elements.urlOut.textContent = newURL;
    elements.urlOutBox.className = '';
    elements.urlButton.textContent = 'Copy URL'
    elements.urlButton.onclick = copyURL;
}, copyURL = () => {
    'use strict';
    const url = elements.urlOut.textContent;
    if (typeof elements.urlOut.setSelectionRange == 'function')
        elements.urlOut.setSelectionRange(0, url.length);
    navigator.clipboard.writeText(url);
    elements.copyAlert.className = '';
    setTimeout(() => elements.copyAlert.className = 'hide-slow');
};

/** @type {(warnings: 0 | 1, disassemble: 0 | 1) => () => void} */
const callInterpreter = (warnings, disassemble) => () => {
    'use strict';

    inputIndex = 0;
    wasmCancel();
    clearOutput();

    elements.disassemble.disabled = true;
    elements.expand.disabled = true;
    elements.condense.disabled = true;

    // By adding a 5ms delay, the previous thread has a chance to clean up after itself.
    // Without this, we may usurp its memory, and then it'll try to deallocate it in a problematic way.
    // Is this a good thing? Probably not. Does it work? Usually. Worst case, just reload the page.
    setTimeout(async () => {
        elements.runStop.textContent = 'Stop';
        elements.runStop.onclick = () => wasmCancel();
        try {
            const result = wasmEntrypoint(elements.program.value, warnings, disassemble, 0);
            if (typeof result == 'object' && 'then' in result)
                await result;
        } catch (e) {
            if (!(e instanceof ExitStatus))
                elements.error.innerText += String(e);
        }
        elements.expand.disabled = false;
        elements.disassemble.disabled = false;
        elements.condense.disabled = false;
        elements.runStop.textContent = 'Run!';
        elements.runStop.onclick = interpretProgram;
    }, 5);
};

const interpretProgram = callInterpreter(1, 0), disassembleProgram = callInterpreter(0, 1);

elements.program.oninput = () => {
    'use strict';
    elements.urlOutBox.className = 'content-hidden';
    elements.urlButton.textContent = 'Generate URL';
    elements.urlButton.onclick = generateURL;
};

// Use an IIFE to prevent polluting the global object
(() => {
    'use strict';  // and also for that

    // Configure the buttons
    const width = elements.runStop.offsetWidth;
    elements.runStop.textContent = 'Run!';
    setTimeout(() => elements.runStop.style.width = `${0.49 + Math.max(width, elements.runStop.offsetWidth)}px`);
    elements.runStop.onclick = interpretProgram;
    elements.urlButton.onclick = generateURL;

    // Get the program from the URL, if present
    if (location.hash.length > 1) {
        elements.program.value = decodeURIComponent(location.hash.slice(1));
    }

    // Set the grid columns and overflow behavior
    const styleEl = document.createElement('style');
    document.head.appendChild(styleEl);
    const emSize = parseFloat(getComputedStyle(document.body).fontSize);
    onresize = () => {
        elements.urlOut.style.width = `${elements.program.clientWidth}px`;
        elements.urlOutBox.style.width = `${elements.program.clientWidth}px`;
        if (elements.main.clientWidth / 2 > elements.program.offsetWidth + emSize)
            styleEl.innerHTML = `.grid { max-height: 100vh; grid-template: "p i" "c ." "o e" / 1fr 1fr; }
.out-container { max-height: calc(100vh - 0.5em - ${
                1.49 + elements.programContainer.offsetHeight + elements.clearContainer.offsetHeight +
                elements.footer.offsetHeight}px); padding-bottom: 1em; }
.out-container :first-child { position: sticky; }\n#stdin { resize: horizontal; }`;
        else
            styleEl.innerHTML = '.grid { grid-template: "p" "i" "c" "o" "e" / 1fr; }';
    };
    new ResizeObserver(onresize).observe(elements.programContainer);
    onresize();
})();
