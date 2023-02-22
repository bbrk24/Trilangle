var inputIndex = 0;
Module = {
    preInit() {
        /**
         * @type {{
         *     input: HTMLTextAreaElement,
         *     output: HTMLParagraphElement,
         *     error: HTMLParagraphElement,
         * }}
         */
        const elements = {
            input: document.getElementById('stdin'),
            output: document.getElementById('stdout'),
            error: document.getElementById('stderr'),
        };

        const decoder = new TextDecoder();
        let stdoutBuffer = [], stderrBuffer = [];

        /** @type {() => (number | null)} */
        const stdin = () => {
            const char = elements.input.value.charCodeAt(inputIndex);
            if (Number.isNaN(char)) {
                return null;
            } else {
                ++inputIndex;
                return char;
            }
        };

        /** @type {(element: HTMLParagraphElement, char: string) => void} */
        const appendChar = (element, char) => {
            if (char === '\n') {
                element.appendChild(document.createElement('br'));
            } else {
                if (element.lastChild instanceof Text) {
                    if (char === ' ' && element.lastChild.nodeValue.endsWith(' ')) {
                        element.lastChild.nodeValue += '\xa0';
                    } else {
                        element.lastChild.nodeValue += char;
                    }
                } else {
                    if (char === ' ') {
                        element.appendChild(document.createTextNode('\xa0'));
                    } else {
                        element.appendChild(document.createTextNode(char));
                    }
                }
            }
        };

        /** @type {(char: number | null) => void} */
        const stdout = (char) => {
            if (typeof char == 'number' && char < 0) {
                stdoutBuffer.push(char);
            } else {
                if (stdoutBuffer.length !== 0) {
                    const typedArray = new Int8Array(stdoutBuffer);
                    stdoutBuffer = [];

                    appendChar(elements.output, decoder.decode(typedArray));
                }

                if (typeof char == 'number')
                    appendChar(elements.output, String.fromCharCode(char));
            }
        };

        /** @type {(char: number | null) => void} */
        const stderr = (char) => {
            if (typeof char == 'number' && char < 0) {
                stderrBuffer.push(char);
            } else {
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
