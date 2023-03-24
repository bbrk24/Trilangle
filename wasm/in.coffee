'use strict'

onFinishHook = null
worker = null
stdoutBuffer = []
stderrBuffer = []
decoder = new TextDecoder

# This is a clever little hack: elements.fooBar is the element with id="foo-bar". It uses document.getElementById on
# first access, but then saves it for fast access later. I've also included the main and footer elements here for
# convenience.
elements = new Proxy 
  main: document.querySelector 'main'
  footer: document.querySelector 'footer'
,
  get: (target, p) ->
    if typeof p == 'string' and not (p of target)
      target[p] = document.getElementById p.replace /[A-Z]/g, (s) => '-' + s.toLowerCase()
    target[p]

@clearOutput = =>
  elements.stdout.innerHTML = ''
  elements.stderr.innerHTML = ''
  
generateContracted = =>
  programText = elements.program.value.replace /^#![^\n]*\n/, ''
    .replace /\n| /g, ''
  # programText.length is wrong when there's high Unicode characters
  programLength = [...programText].length
  # Calculate the largest triangular number less than the length.
  # minLength is 1 more than that
  temp = Math.ceil Math.sqrt(2 * programLength) - 1.5
  minLength = 1 + temp * (temp + 1) / 2
  if programLength > minLength
    programText = programText.replace new RegExp("\\.{0,#{programLength - minLength}}$"), ''
  programText.replace /^#!/, '#\n!'

@contractInput = =>
  clearOutput()
  elements.program.value = generateContracted()
  
generateURL = => 
  newURL = "#{location.href.split('#')[0]}\##{encodeURIComponent(generateContracted())}"
  history.pushState {}, '', newURL
  elements.urlOut.textContent = newURL
  elements.urlOutBox.className = ''
  elements.urlButton.textContent = 'Copy URL'
  elements.urlButton.onclick = copyURL
  
copyURL = =>
  url = elements.urlOut.textContent
  elements.urlOut.setSelectionRange? 0, url.length
  navigator.clipboard.writeText url
  elements.copyAlert.className = ''
  setTimeout => elements.copyAlert.className = 'hide-slow'
  
workerFinished = =>
  elements.expand.disabled = false
  elements.disassemble.disabled = false
  elements.condense.disabled = false
  elements.runStop.textContent = 'Run!'
  elements.runStop.onclick = interpretProgram
  onFinishHook?()
  
wasmCancel = =>
  worker?.terminate()
  worker = null
  workerFinished()
  
isBufferFull = (buf) => (
  buf.length == 4 || 
  (((buf[0] + 256) & 0xf0) == 0xe0 && buf.length == 3) ||
  (((buf[0] + 256) & 0xe0) == 0xc0 && buf.length == 2)  
)

stdout = (char) =>
  return unless char?
  if char < 0
    stdoutBuffer.push char
    if isBufferFull stdoutBuffer
      elements.stdout.textContent += decoder.decode new Int8Array stdoutBuffer
      stdoutBuffer = []
  else
    elements.stdout.textContent += String.fromCharCode char

stderr = (char) =>
  return unless char?
  appendText = (text) =>
    if elements.stderr.lastChild instanceof Text
      elements.stderr.lastChild.nodeValue += text
    else
      elements.stderr.appendChild document.createTextNode text

  if char < 0
    stderrBuffer.push char
    if isBufferFull stderrBuffer
      appendText decoder.decode new Int8Array stderrBuffer
      stderrBuffer = []
  else if char == 10
    elements.stderr.appendChild document.createElement 'br'
  else
    appendText String.fromCharCode char
  
createWorker = (name) => =>
  clearOutput()
  elements.disassemble.disabled = true
  elements.expand.disabled = true
  elements.condense.disabled = true
  elements.runStop.textContent = 'Stop'
  elements.runStop.onclick = wasmCancel
  worker ?= new Worker new URL 'worker.js', location
  worker.onmessage = (event) ->
    content = event.data[1]
    switch event.data[0]
      when 0 then workerFinished()
      when 1
        if typeof content == 'string'
          elements.stdout.textContent += content
        else
          stdout content
      when 2
        if typeof content == 'string'
          elements.stderr.innerText += content
        else
          stderr content
      else console.error "Unrecognized destination #{event.data[0]}"
  worker.postMessage [name, elements.program.value, elements.stdin.value]
  
interpretProgram = createWorker 'interpretProgram'
@disassembleProgram = createWorker 'disassembleProgram'
expandBase = createWorker 'expandInput'

@expandInput = =>
  onFinishHook = =>
    elements.program.value = elements.stdout.innerText
    elements.stdout.innerText = ''
    onFinishHook = null
  expandBase()
  
elements.program.oninput = ->
  elements.urlOutBox.className = 'content-hidden'
  elements.urlButton.textContent = 'Generate URL'
  elements.urlButton.onclick = generateURL
  
width = elements.runStop.offsetWidth
remSize = parseFloat getComputedStyle(document.body).fontSize
elements.runStop.textContent = 'Run!'
setTimeout => elements.runStop.style.width = "#{(0.49 + Math.max width, elements.runStop.offsetWidth) / remSize}"
elements.runStop.onclick = interpretProgram
elements.urlButton.onclick = generateURL

if location.hash.length > 1
  elements.program.value = decodeURIComponent location.hash.slice 1
