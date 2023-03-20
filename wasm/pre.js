// JSDoc comments with type info are used by the optimizer. I wouldn't otherwise include all of them.

let inputIndex = 0, ready = false,
    /** @type {Uint8Array} */
    stdinBuffer,
    /** @type {string} */
    programText,
    /** @type {?string} */
    funcName = null;

const encoder = new TextEncoder(),
      /** @type {Map.<string, Function>} */
    signals = new Map(),
      /** @param {string=} arg */
    halfReady = arg => {
        'use strict';
        funcName = arg ?? funcName;
        if (ready) {
            const signal = signals.get(funcName);
            if (typeof signal == 'undefined')
                console.error('Unrecognized signal ' + funcName);
            else
                signal();
            self.postMessage([0, null]);
        } else
            ready = true;
    };

Module = {
    'preInit': () => {
        'use strict';

        const stdin = () => {
            if (inputIndex >= stdinBuffer.length)
                return null;

            return stdinBuffer.at(inputIndex++);
        }, stdout = char => self.postMessage([1, char]), stderr = char => self.postMessage([2, char]);

        FS.init(stdin, stdout, stderr);
    },
    'onRuntimeInitialized': () => {
        halfReady();
    },
    'noExitRuntime': true,
};

/**
 * @param {number} warnings
 * @param {number} disassemble
 * @param {number} expand
 */
const callInterpreter = (warnings, disassemble, expand) => () => {
    'use strict';

    try {
        Module['ccall'](
            'wasm_entrypoint',
            null,
            ['string', 'number', 'number', 'number'],
            [programText, warnings, disassemble, expand]
        );
    } catch (e) {
        if (!(e instanceof ExitStatus))
            self.postMessage([2, String(e)]);
    }
};

signals.set('interpretProgram', callInterpreter(1, 0, 0));
signals.set('disassembleProgram', callInterpreter(0, 1, 0));
signals.set('expandInput', callInterpreter(0, 0, 1));

onmessage = ({ data: [func, program, input] }) => {
    stdinBuffer = encoder.encode(input);
    programText = program;
    halfReady(func);
};
