var inputIndex = 0;
Module = {
    preInit() {
        /** @type {{ input: HTMLTextAreaElement, output: HTMLPreElement, error: HTMLParagraphElement }} */
        const elements = {
            input: document.getElementById('stdin'),
            output: document.getElementById('stdout'),
            error: document.getElementById('stderr'),
        };

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

        /** @type {(element: HTMLParagraphElement, char: string) => void} */
        const appendChar = (element, char) => {
            if (char === '\n')
                element.appendChild(document.createElement('br'));
            else if (element.lastChild instanceof Text) {
                if (char === ' ' && element.lastChild.nodeValue.endsWith(' '))
                    element.lastChild.nodeValue += '\xa0';
                else
                    element.lastChild.nodeValue += char;
            } else
                element.appendChild(document.createTextNode(char === ' ' ? '\xa0' : char));
        };

        /** @type {(char: number | null) => void} */
        const stdout = (char) => {
            if (typeof char == 'number') {
                if (char < 0) {
                    stdoutBuffer.push(char);

                    if (
                        stdoutBuffer.length === 4 ||
                        ((stdoutBuffer[0] + 256) & 0xf0 === 0xe0 && stdoutBuffer.length >= 3) ||
                        ((stdoutBuffer[0] + 256) & 0xe0 === 0xc0 && stdoutBuffer.length >= 2)
                    ) {
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

                    if (
                        stderrBuffer.length === 4 ||
                        ((stderrBuffer[0] + 256) & 0xf0 === 0xe0 && stderrBuffer.length >= 3) ||
                        ((stderrBuffer[0] + 256) & 0xe0 === 0xc0 && stderrBuffer.length >= 2)
                    ) {
                        appendChar(elements.error, decoder.decode(new Int8Array(stderrBuffer)));
                        stderrBuffer = [];
                    }
                } else
                    appendChar(elements.error, String.fromCharCode(char));
            }
        };

        FS.init(stdin, stdout, stderr);
    },
    noExitRuntime: true,
};
