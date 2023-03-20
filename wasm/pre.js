// JSDoc comments with type info are used by the optimizer. I wouldn't otherwise include all of them.

let inputIndex = 0,
    /** @type {Uint8Array} */
    stdinBuffer,
    /** @type {string} */
    programText, ready = false,
    /** @type {?string} */
    funcName = null;

/** @param {=string} arg */
const halfReady = arg => {
    'use strict';
    funcName = arg ?? funcName;
    if (ready) {
        switch (funcName) {
            case 'interpretProgram':
                interpretProgram();
                break;
            case 'disassembleProgram':
                disassembleProgram();
                break;
            case 'expandInput':
                expandInput();
                break;
            default:
                console.error(`Unrecognized selector ${funcName} sent to worker`);
                break;
        }
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
    noExitRuntime: true,
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

/** @function */
const interpretProgram = callInterpreter(1, 0, 0),
      /** @function */
    disassembleProgram = callInterpreter(0, 1, 0),
      /** @function */
    expandInput = callInterpreter(0, 0, 1), encoder = new TextEncoder();

onmessage = ({ data: [func, program, input] }) => {
    stdinBuffer = encoder.encode(input);
    programText = program;
    halfReady(func);
};
