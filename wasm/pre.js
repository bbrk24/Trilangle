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
            if (typeof char == 'number' && char < 0)
                stdoutBuffer.push(char);
            else {
                if (stdoutBuffer.length !== 0) {
                    const typedArray = new Int8Array(stdoutBuffer);
                    stdoutBuffer = [];

                    elements.output.textContent += decoder.decode(typedArray);
                }

                if (typeof char == 'number')
                    elements.output.textContent += String.fromCharCode(char);
            }
        };

        /** @type {(char: number | null) => void} */
        const stderr = (char) => {
            if (typeof char == 'number' && char < 0)
                stderrBuffer.push(char);
            else {
                if (stderrBuffer.length !== 0) {
                    const typedArray = new Int8Array(stderrBuffer);
                    stderrBuffer = [];

                    appendChar(elements.error, decoder.decode(typedArray));
                }

                if (typeof char == 'number')
                    appendChar(elements.error, String.fromCharCode(char));
            }
        };

        FS.init(stdin, stdout, stderr);
    },
    noExitRuntime: true,
};
