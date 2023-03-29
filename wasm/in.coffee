'use strict'

onFinishHook = null
worker = null
stdoutBuffer = []
stderrBuffer = []
decoder = new TextDecoder

# This is a clever little hack: elements.fooBar is the element with id="foo-bar". It uses document.getElementById on
# first access, but then saves it for fast access later.
elements = new Proxy {}, get: (target, p) ->
  if typeof p is 'string' and not (p of target)
    target[p] = document.getElementById p.replace /[A-Z]/g, (s) => '-' + s.toLowerCase()
  target[p]

@clearOutput = =>
  elements.stdout.innerHTML = ''
  elements.stderr.innerHTML = ''
  
generateContracted = =>
  programText = elements.program.value.replace /^#![^\n]*\n/, ''
    .replace /\n| /g, ''
  # programText.length is wrong when there's high Unicode characters
  programLength = [programText...].length
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
  newURL = "#{location.href.split('#')[0]}\##{encodeURIComponent generateContracted()}"
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
  elements.stdin.oninput = ->
    worker = null
    @oninput = null
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
  
isBufferFull = (buf) =>
  buf.length is 4 or
  (((buf[0] + 256) & 0xf0) is 0xe0 and buf.length is 3) or
  (((buf[0] + 256) & 0xe0) is 0xc0 and buf.length is 2)

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
  else if char is 10
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
  elements.stdin.oninput = null
  worker ?= new Worker new URL 'worker.js', location
  worker.onmessage = (event) ->
    content = event.data[1]
    switch event.data[0]
      when 0 then workerFinished()
      when 1
        if typeof content is 'string'
          elements.stdout.textContent += content
        else
          stdout content
      when 2
        if typeof content is 'string'
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

elements.darkSwitch.onchange = ->
  if @checked
    document.body.classList.add 'dark'
  else
    document.body.classList.remove 'dark'
  localStorage.setItem 'dark-mode', if @checked then 'true' else 'false'

elements.contrastSwitch.onchange = ->
  if @checked
    document.body.classList.add 'high-contrast'
  else
    document.body.classList.remove 'high-contrast'
  localStorage.setItem 'high-contrast', if @checked then 'true' else 'false'
  
width = elements.runStop.offsetWidth
remSize = parseFloat getComputedStyle(document.body).fontSize
elements.runStop.textContent = 'Run!'
setTimeout => elements.runStop.style.width = "#{(0.49 + Math.max width, elements.runStop.offsetWidth) / remSize}rem"
elements.runStop.onclick = interpretProgram
elements.urlButton.onclick = generateURL

if location.hash.length > 1
  elements.program.value = decodeURIComponent location.hash.slice 1
