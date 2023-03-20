'use strict';

let inputIndex = 0, stdoutBuffer = [], stderrBuffer = [], worker = null, onFinishHook = null;

const elements = new Proxy({ footer: document.querySelector('footer'), main: document.querySelector('main') }, {
    get(target, p) {
        if (typeof p == 'string' && !(p in target)) {
            const id = p.replace(/[A-Z]/g, s => '-' + s.toLowerCase());
            target[p] = document.getElementById(id);
        }
        return target[p];
    },
}),
      decoder = new TextDecoder();

const contractInput = () => {
    clearOutput();
    elements.program.value = generateContracted();
}, clearOutput = () => {
    elements.stdout.innerHTML = '';
    elements.stderr.innerHTML = '';
}, generateContracted = () => {
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
    const newURL = `${location.href.split('#')[0]}#${encodeURIComponent(generateContracted())}`;
    history.pushState({}, '', newURL);
    elements.urlOut.textContent = newURL;
    elements.urlOutBox.className = '';
    elements.urlButton.textContent = 'Copy URL'
    elements.urlButton.onclick = copyURL;
}, copyURL = () => {
    const url = elements.urlOut.textContent;
    if (typeof elements.urlOut.setSelectionRange == 'function')
        elements.urlOut.setSelectionRange(0, url.length);
    navigator.clipboard.writeText(url);
    elements.copyAlert.className = '';
    setTimeout(() => elements.copyAlert.className = 'hide-slow');
}, stdout = char => {
    if (typeof char == 'number') {
        if (char < 0) {
            stdoutBuffer.push(char);

            if (stdoutBuffer.length === 4 || (((stdoutBuffer[0] + 256) & 0xf0) === 0xe0 && stdoutBuffer.length === 3) ||
                (((stdoutBuffer[0] + 256) & 0xe0) === 0xc0 && stdoutBuffer.length === 2)) {
                elements.stdout.textContent += decoder.decode(new Int8Array(stdoutBuffer));
                stdoutBuffer = [];
            }
        } else
            elements.stdout.textContent += String.fromCharCode(char);
    }
}, stderr = char => {
    if (typeof char == 'number') {
        if (char < 0) {
            stderrBuffer.push(char);

            if (stderrBuffer.length === 4 || (((stderrBuffer[0] + 256) & 0xf0) === 0xe0 && stderrBuffer.length >= 3) ||
                (((stderrBuffer[0] + 256) & 0xe0) === 0xc0 && stderrBuffer.length >= 2)) {
                const text = decoder.decode(new Int8Array(stderrBuffer));
                stderrBuffer = [];
                if (elements.stderr.lastChild instanceof Text)
                    elements.stderr.lastChild.nodeValue += text;
                else
                    elements.stderr.appendChild(document.createTextNode(text));
            }
        } else if (char === 10)
            elements.stderr.appendChild(document.createElement('br'));
        else if (elements.stderr.lastChild instanceof Text) {
            if (char === 32 && elements.stderr.lastChild.nodeValue.endsWith(' '))
                elements.stderr.lastChild.nodeValue += '\xa0';
            else
                elements.stderr.lastChild.nodeValue += String.fromCharCode(char);
        } else
            elements.stderr.appendChild(document.createTextNode(char === 32 ? '\xa0' : String.fromCharCode(char)));
    }
}, wasmCancel = () => {
    worker?.terminate();
    workerFinished();
}, createWorker = name => () => {
    clearOutput();

    elements.disassemble.disabled = true;
    elements.expand.disabled = true;
    elements.condense.disabled = true;

    elements.runStop.textContent = 'Stop';
    elements.runStop.onclick = wasmCancel;

    worker = new Worker('out.js');
    worker.onmessage = ({ data: [fd, content] }) => {
        switch (fd) {
            case 0:
                workerFinished();
                break;
            case 1:
                if (typeof content == 'string')
                    elements.stdout.textContent += content;
                else
                    stdout(content);
                break;
            case 2:
                if (typeof content == 'string')
                    elements.stderr.innerText += content;
                else
                    stderr(content);
                break;
            default:
                console.error(`Unrecognized destination ${fd}`);
        }
    };
    worker.postMessage([name, elements.program.value, elements.stdin.value]);
}, workerFinished = () => {
    elements.expand.disabled = false;
    elements.disassemble.disabled = false;
    elements.condense.disabled = false;
    elements.runStop.textContent = 'Run!';
    elements.runStop.onclick = interpretProgram;
    onFinishHook?.();
}, expandInput = () => {
    onFinishHook = () => {
        elements.program.value = elements.stdout.innerText;
        elements.stdout.innerHTML = '';
        onFinishHook = null;
    };
    expandBase();
};

const interpretProgram = createWorker('interpretProgram'), disassembleProgram = createWorker('disassembleProgram'),
      expandBase = createWorker('expandInput');

elements.program.oninput = () => {
    elements.urlOutBox.className = 'content-hidden';
    elements.urlButton.textContent = 'Generate URL';
    elements.urlButton.onclick = generateURL;
};

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