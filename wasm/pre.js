let inputIndex = 0, stdinBuffer, programText, ready = false, funcName = null;

const halfReady = arg => {
    funcName = arg ?? funcName;
    if (ready)
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
    else
        ready = true;
};

Module = {
    preInit() {
        'use strict';

        const stdin = () => {
            if (inputIndex >= stdinBuffer.length)
                return null;

            return stdinBuffer.at(inputIndex++);
        }, stdout = char => self.postMessage([1, char]), stderr = char => self.postMessage([2, char]);

        FS.init(stdin, stdout, stderr);
    },
    onRuntimeInitialized() {
        halfReady(null);
    },
};

const callInterpreter = (warnings, disassemble, expand) => () => {
    'use strict';

    try {
        Module.ccall(
            'wasm_entrypoint',
            null,
            ['string', 'number', 'number', 'number'],
            [programText, warnings, disassemble, expand]
        );
    } catch (e) {
        if (!(e instanceof ExitStatus))
            self.postMessage([2, String(e)]);
    }

    self.postMessage([0, null]);
};

const interpretProgram = callInterpreter(1, 0, 0), disassembleProgram = callInterpreter(0, 1, 0),
      expandInput = callInterpreter(0, 0, 1), encoder = new TextEncoder();

onmessage = ({ data: [func, program, input] }) => {
    stdinBuffer = encoder.encode(input);
    programText = program;
    halfReady(func);
};
