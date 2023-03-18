'use strict';
const expandInput = () => {
    clearOutput();
    wasmEntrypoint(elements.program.value, 0, 0, 1);
    elements.program.value = elements.output.innerText;
    elements.output.innerHTML = '';
}, contractInput = () => {
    clearOutput();
    elements.program.value = generateContracted();
};
