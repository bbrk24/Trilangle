'use strict';
const generateContracted = () => {
    // Remove an unnecessary shebang
    let programText = elements.program.value.replace(/^#![^\n]*\n/, '');
    // Remove spaces and newlines (intentionally not other whitespace)
    programText = programText.replace(/ |\n/g, '');
    // programText.length is wrong when there's high Unicode characters
    const programLength = [...programText].length;
    // Calculate the largest triangular number less than the length. minLength is 1 more than that
    const temp = Math.ceil(Math.sqrt(2 * programLength) - 1.5);
    const minLength = 1 + temp * (temp + 1) / 2;
    // Remove trailing dots, but not too many
    if (programLength !== minLength) {
        programText = programText.replace(new RegExp(`\\.{0,${programLength - minLength}}\$`), '');
    }
    // Fix an accidentally-created shebang
    programText = programText.replace(/^#!/, '#\n!');
    return programText;
}, expandInput = () => {
    clearOutput();
    wasmEntrypoint(elements.program.value, 0, 0, 1);
    elements.program.value = elements.output.innerText;
    elements.output.innerHTML = '';
}, contractInput = () => {
    clearOutput();
    elements.program.value = generateContracted();
}, generateURL = () => {
    const newURL = `${location.href.split('#')[0]}#${encodeURIComponent(generateContracted())}`;
    history.pushState({}, '', newURL);
    elements.urlOut.textContent = newURL;
    elements.urlOutBox.className = '';
    elements.urlButton.textContent = 'Copy URL'
}, copyURL = () => {
    const url = elements.urlOut.textContent;
    elements.urlOut.setSelectionRange?.(0, url.length);
    navigator.clipboard.writeText(url);
    elements.copyAlert.className = '';
    setTimeout(() => elements.copyAlert.className = 'hide-slow');
}, runStopClicked = () => {
    if (elements.disassembleButton.disabled)
        wasmCancel();
    else
        interpretProgram();
}, urlButtonClicked = () => {
    if (elements.urlButton.textContent === 'Copy URL')
        copyURL();
    else
        generateURL();
};
