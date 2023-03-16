'use strict';
const generateContracted = () => {
    // Remove spaces and newlines (intentionally not other whitespace)
    const programText = elements.program.value.replace(/ |\n/g, '');
    // programText.length is wrong when there's high Unicode characters
    const programLength = [...programText].length;
    // Calculate the largest triangular number less than the length. minLength is 1 more than that
    const temp = Math.ceil(Math.sqrt(2 * programLength) - 1.5);
    const minLength = 1 + temp * (temp + 1) / 2;
    // Remove trailing dots, but not too many
    if (programLength !== minLength) {
        return programText.replace(new RegExp(`\\.{0,${programLength - minLength}}\$`), '');
    }
    return programText;
}, expandInput = () => {
    clearOutput();
    Module.ccall('wasm_entrypoint', null, ['string', 'number', 'number', 'number'], [elements.program.value, 0, 0, 1]);
    elements.program.value = elements.output.innerText;
    elements.output.innerHTML = '';
}, contractInput = () => {
    clearOutput();
    elements.program.value = generateContracted();
}, generateURL = () => {
    const newURL = `${location.href.split('?')[0]}?p=${encodeURIComponent(generateContracted())}`;
    history.pushState({}, '', newURL);
    elements.urlOut.textContent = newURL;
    elements.urlOutBox.className = '';
}, copyURL = () => {
    const url = elements.urlOut.textContent;
    elements.urlOut.setSelectionRange?.(0, url.length);
    navigator.clipboard.writeText(url);
    elements.copyAlert.className = '';
    setTimeout(() => elements.copyAlert.className = 'hide-slow');
};
