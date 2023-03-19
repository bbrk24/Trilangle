let inputIndex = 0;

const elements = new Proxy({ footer: document.querySelector('footer'), main: document.querySelector('main') }, {
    get(target, p) {
        'use strict';
        if (typeof p == 'string' && !(p in target)) {
            const id = p.replace(/[A-Z]/g, s => '-' + s.toLowerCase());
            target[p] = document.getElementById(id);
        }
        return target[p];
    },
});

Module = {
    preInit() {
        'use strict';

        const encoder = new TextEncoder(), decoder = new TextDecoder();
        let stdoutBuffer = [], stderrBuffer = [];
        /** @type {Uint8Array?} */
        let stdinBuffer = null;

        const stdin = () => {
            if (inputIndex === 0)
                stdinBuffer = encoder.encode(elements.stdin.value);

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
                        elements.stdout.textContent += decoder.decode(new Int8Array(stdoutBuffer));
                        stdoutBuffer = [];
                    }
                } else
                    elements.stdout.textContent += String.fromCharCode(char);
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
                    elements.stderr.appendChild(
                        document.createTextNode(char === 32 ? '\xa0' : String.fromCharCode(char))
                    );
            }
        };

        FS.init(stdin, stdout, stderr);
    },
    noExitRuntime: true,
};
