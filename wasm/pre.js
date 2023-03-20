// JSDoc comments with type info are used by the optimizer. I wouldn't otherwise include all of them.

let /** number */ inputIndex = 0, /** boolean */ ready = false;
/** @type {Uint8Array} */
let stdinBuffer;
/** @type {string} */
let programText;
/** @type {?string} */
let funcName = null;

const encoder = new TextEncoder();
/** @type {!Map.<string, function():void>} */
const signals = new Map();
/** @param {string=} arg */
const halfReady = arg => {
    funcName = arg ?? funcName;
    if (ready) {
        /** @type {undefined|function():void} */
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
        /** @returns {?number} */
        const stdin = () => {
            if (inputIndex >= stdinBuffer.length)
                return null;

            return stdinBuffer.at(inputIndex++);
        };
        /** @param {?number} char */
        const stdout = char => self.postMessage([1, char]);
        /** @param {?number} char */
        const stderr = char => self.postMessage([2, char]);

        FS.init(stdin, stdout, stderr);
    },
    'onRuntimeInitialized': halfReady,
    'noExitRuntime': true,
};

/**
 * @param {number} warnings
 * @param {number} disassemble
 * @param {number} expand
 * @returns {function():void}
 */
const callInterpreter = (warnings, disassemble, expand) => () => {
    inputIndex = 0;
    try {
        Module['ccall'](
            'wasm_entrypoint',
            null,
            ['string', 'number', 'number', 'number'],
            [programText, warnings, disassemble, expand]
        );
    } catch (/** string|Error= */ e) {
        if (typeof e != 'undefined' && !(e instanceof ExitStatus))
            self.postMessage([2, e.toString()]);
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
