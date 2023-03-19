'use strict';
const expandInput = () => {
    clearOutput();
    wasmEntrypoint(elements.program.value, 0, 0, 1);
    elements.program.value = elements.stdout.innerText;
    elements.stdout.innerHTML = '';
}, contractInput = () => {
    clearOutput();
    elements.program.value = generateContracted();
};
