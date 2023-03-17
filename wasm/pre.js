let inputIndex = 0;

/**
 * @type {{
 *  input: HTMLTextAreaElement,
 *  output: HTMLPreElement,
 *  error: HTMLParagraphElement,
 *  program: HTMLTextAreaElement,
 *  urlOutBox: HTMLDivElement,
 *  urlOut: HTMLCodeElement,
 *  copyAlert: HTMLSpanElement,
 *  urlButton: HTMLButtonElement,
 *  programFieldset: HTMLFieldSetElement,
 *  clearContainer: HTMLDivElement,
 *  footer: HTMLFooterElement,
 *  outputLabel: HTMLDivElement,
 *  runStopButton: HTMLButtonElement,
 *  disassembleButton: HTMLButtonElement,
 *  expandButton: HTMLButtonElement,
 * }}
 */
const elements = {
    input: document.getElementById('stdin'),
    output: document.getElementById('stdout'),
    error: document.getElementById('stderr'),
    program: document.getElementById('program'),
    urlOutBox: document.getElementById('url-out-box'),
    urlOut: document.getElementById('url-out'),
    copyAlert: document.getElementById('copy-alert'),
    urlButton: document.getElementById('url-button'),
    programFieldset: document.getElementById('program-container'),
    clearContainer: document.getElementById('clear-button-container'),
    footer: document.querySelector('footer'),
    outputLabel: document.getElementById('out-label'),
    runStopButton: document.getElementById('run-stop'),
    disassembleButton: document.getElementById('disassemble'),
    expandButton: document.getElementById('expand'),
};

Module = {
    preInit() {
        'use strict';

        const encoder = new TextEncoder(), decoder = new TextDecoder();
        let stdoutBuffer = [], stderrBuffer = [];
        /** @type {Uint8Array?} */
        let stdinBuffer = null;

        const stdin = () => {
            if (inputIndex === 0)
                stdinBuffer = encoder.encode(elements.input.value);

            if (inputIndex >= stdinBuffer.length)
                return null;

            return stdinBuffer.at(inputIndex++);
        };

        /** @type {(char: number | null) => void} */
        const stdout = (char) => {
            if (typeof char == 'number') {
                if (char < 0) {
                    stdoutBuffer.push(char);

                    if (stdoutBuffer.length === 4 ||
                        (((stdoutBuffer[0] + 256) & 0xf0) === 0xe0 && stdoutBuffer.length === 3) ||
                        (((stdoutBuffer[0] + 256) & 0xe0) === 0xc0 && stdoutBuffer.length === 2)) {
                        elements.output.textContent += decoder.decode(new Int8Array(stdoutBuffer));
                        stdoutBuffer = [];
                    }
                } else
                    elements.output.textContent += String.fromCharCode(char);
            }
        };

        /** @type {(char: number | null) => void} */
        const stderr = (char) => {
            if (typeof char == 'number') {
                if (char < 0) {
                    stderrBuffer.push(char);

                    if (stderrBuffer.length === 4 ||
                        (((stderrBuffer[0] + 256) & 0xf0) === 0xe0 && stderrBuffer.length >= 3) ||
                        (((stderrBuffer[0] + 256) & 0xe0) === 0xc0 && stderrBuffer.length >= 2)) {
                        const text = decoder.decode(new Int8Array(stderrBuffer));
                        stderrBuffer = [];
                        if (elements.error.lastChild instanceof Text)
                            elements.error.lastChild.nodeValue += text;
                        else
                            elements.error.appendChild(document.createTextNode(text));
                    }
                } else if (char === 10)
                    elements.error.appendChild(document.createElement('br'));
                else if (elements.error.lastChild instanceof Text) {
                    if (char === 32 && elements.error.lastChild.nodeValue.endsWith(' '))
                        elements.error.lastChild.nodeValue += '\xa0';
                    else
                        elements.error.lastChild.nodeValue += String.fromCharCode(char);
                } else
                    elements.error.appendChild(document.createTextNode(char === 32 ? '\xa0' : String.fromCharCode(char))
                    );
            }
        };

        FS.init(stdin, stdout, stderr);
    },
    noExitRuntime: true,
};
