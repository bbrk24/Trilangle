'use strict'

worker = null
stdoutBuffer = []
stderrBuffer = []
threads = []
decoder = new TextDecoder
resolve = null
delay = 500
interval = -1

# A list of colors used for threads.
# TODO: Should these be customizable?
threadColors = [
  'rgb(0, 173, 0)',
  'rgb(255, 40, 255)',
  'rgb(49, 151, 255)',
  'rgb(187, 142, 29)',
  'rgb(255, 93, 94)',
  'rgb(28, 166, 159)',
  'rgb(199, 117, 219)',
  'rgb(255, 97, 38)',
]

# A mapping of color -> div element.
highlights = {}

# A mapping of thread number -> color.
usedColors = []

# Temporarily remove a single color, but keep the div present in highlights for later use.
removeHighlight = (color) ->
  if highlights[color]?
    highlights[color].remove()
    usedColors[usedColors.indexOf color] = null

# Get an available color for the thread number.
getColor = (idx) ->
  usedColors[idx] ?= threadColors.find (color) -> not (color in usedColors)

# Destroy all highlight divs, freeing them for the garbage collector.
removeAllHighlights = ->
  highlights = {}
  usedColors = []

# This is a clever little hack: elements.fooBar is the element with id="foo-bar". It uses document.getElementById on
# first access, but then saves it for fast access later.
elements = new Proxy {}, get: (target, p) ->
  if typeof p is 'string' and not (p of target)
    target[p] = document.getElementById p.replace /[A-Z]/g, (s) -> '-' + s.toLowerCase()
  target[p]

@clearOutput = ->
  elements.stdout.innerHTML = ''
  elements.stderr.innerHTML = ''

generateContracted = ->
  programText = elements.program.value.replace /^#![^\n]*\n/u, ''
    .replace /\n| /gu, ''
  # programText.length is wrong when there's high Unicode characters
  programLength = [programText...].length
  # Calculate the largest triangular number less than the length.
  # minLength is 1 more than that
  temp = Math.ceil Math.sqrt(2 * programLength) - 1.5
  minLength = 1 + temp * (temp + 1) / 2
  if programLength > minLength
    programText = programText.replace new RegExp("\\.{0,#{programLength - minLength}}$"), ''
  programText.replace /^#!/u, '#\n!'

@contractInput = ->
  clearOutput()
  elements.program.value = generateContracted()

generateURL = ->
  baseURL = location.href.split(/[?#]/u)[0]
  fragment = '#' + encodeURIComponent generateContracted()
  query = if elements.includeInput.checked then "?i=#{encodeURIComponent elements.stdin.value}" else ''
  newURL = "#{baseURL}#{query}#{fragment}"
  history.pushState {}, '', newURL
  elements.urlOut.textContent = newURL
  elements.urlOutBox.className = ''
  elements.urlButton.textContent = 'Copy URL'
  elements.urlButton.onclick = copyURL

copyURL = ->
  url = elements.urlOut.textContent
  elements.urlOut.setSelectionRange? 0, url.length
  navigator.clipboard.writeText url
  elements.copyAlert.className = ''
  setTimeout -> elements.copyAlert.className = 'hide-slow'

wasmCancel = ->
  worker?.terminate()
  worker = null
  resolve()

isBufferFull = (buf) ->
  buf.length is 4 or
  (((buf[0] + 256) & 0xf0) is 0xe0 and buf.length is 3) or
  (((buf[0] + 256) & 0xe0) is 0xc0 and buf.length is 2)

stdout = (char) ->
  return unless char?
  if char < 0
    stdoutBuffer.push char
    if isBufferFull stdoutBuffer
      elements.stdout.textContent += decoder.decode new Int8Array stdoutBuffer
      stdoutBuffer = []
  else
    elements.stdout.textContent += String.fromCharCode char

stderr = (char) ->
  return unless char?
  appendText = (text) ->
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
  threadCount = -1

  clearOutput()

  elements.disassemble.disabled = true
  elements.expand.disabled = true
  elements.condense.disabled = true
  elements.debug.disabled = true
  elements.runStop.textContent = 'Stop'
  elements.runStop.onclick = wasmCancel
  elements.stdin.oninput = null

  worker ?= new Worker @WORKER_URL
  @step = -> worker.postMessage ['step']

  promise = new Promise (r) -> resolve = r
    .then ->
      elements.stdin.oninput = ->
        worker = null
        @oninput = null
      elements.expand.disabled = false
      elements.disassemble.disabled = false
      elements.condense.disabled = false
      elements.debug.disabled = false
      elements.runStop.textContent = 'Run!'
      elements.runStop.onclick = interpretProgram

  worker.onmessage = (event) ->
    content = event.data[1]
    switch event.data[0]
      when 0 then resolve()
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
      when 3
        if typeof content is 'number'
          threadCount = content
          threads = []
        else
          threads[content[0]] = content[1]
          if threadCount is threads.reduce (x) -> x + 1, 0
            renderThreads()
          else
            step()
      else console.error "Unrecognized destination #{event.data[0]}", content
  worker.postMessage [name, elements.program.value, elements.stdin.value]
  
  return promise

interpretProgram = createWorker 'interpretProgram'
@disassembleProgram = createWorker 'disassembleProgram'
expandBase = createWorker 'expandInput'
debugBase = createWorker 'debugProgram'

@expandInput = ->
  await expandBase()
  elements.program.value = elements.stdout.innerText
  elements.stdout.innerText = ''

pause = =>
  clearInterval interval
  interval = -1
  elements.playPause.textContent = @PLAY_TEXT
  elements.slower.disabled = true
  elements.faster.disabled = true
  elements.step.disabled = false

# At some point, calling `step` doesn't help any. Besides, browsers throttle requests that are too fast.
minDelay = 4
# Not really a useful upper limit but one that keeps setInterval from breaking
maxDelay = 1 << 30
play = =>
  elements.playPause.textContent = @PAUSE_TEXT
  elements.slower.disabled = delay >= maxDelay
  elements.faster.disabled = delay <= minDelay
  elements.step.disabled = true
  interval = setInterval step, delay

Object.defineProperty @, 'isPaused', get: -> interval is -1

@playPause = =>
  if @isPaused then play() else pause()

@faster = ->
  clearInterval interval
  delay /= 2
  interval = setInterval step, delay
  elements.slower.disabled = false
  elements.faster.disabled = delay <= minDelay

@slower = ->
  clearInterval interval
  delay *= 2
  interval = setInterval step, delay
  elements.faster.disabled = false
  elements.slower.disabled = delay >= maxDelay

@debugProgram = =>
  await expandBase()

  elements.debugProgram.hidden = false
  elements.debugProgram.style.width = "#{elements.program.offsetWidth}px"
  elements.debugProgram.style.minHeight = "#{elements.program.offsetHeight}px"
  elements.program.hidden = true

  elements.stdout.innerText.trimEnd().split '\n'
    .forEach (textRow, i) ->
      el = document.createElement 'div'
      el.id = "row-#{i}"
      # Codegen is just stupid surrounding null-coalescing and null-chaining operators in the same expression
      match = /^ */u.exec textRow
      matchLength = if match? then match[0].length else 0
      el.dataset.offset = String matchLength
      el.style.paddingLeft = "#{matchLength}ch"
      textRow.trimStart().split ' '
        .forEach (char, i) ->
          span = document.createElement 'span'
          span.textContent = char
          el.appendChild span
      elements.debugProgram.appendChild el
  elements.stdout.innerText = ''

  elements.debugInfo.hidden = false
  if not elements.playPause.style?.width
    elements.playPause.textContent = @PAUSE_TEXT
    setTimeout ->
      elements.playPause.style.width = "#{elements.playPause.offsetWidth}px"
      pause()

  await debugBase()

  removeAllHighlights()
  elements.debugInfo.hidden = true
  elements.debugProgram.innerHTML = ''
  elements.debugProgram.hidden = true
  elements.program.hidden = false
  pause()

renderThreads = ->
  for row in elements.threads.children
    unless row.firstChild.textContent of threads
      removeHighlight row.style.backgroundColor
  elements.threads.innerHTML = ''
  threads.forEach (thread, idx) ->
    row = document.createElement 'tr'
    color = getColor idx
    row.style.backgroundColor = color
    highlightIndex thread.x, thread.y, color
    threadIdEl = document.createElement 'td'
    threadIdEl.textContent = String idx
    row.appendChild threadIdEl
    threadContentsEl = document.createElement 'td'
    threadContentsEl.textContent = thread.stack.join ',\u2009'
    row.appendChild threadContentsEl
    elements.threads.appendChild row

# Given y,x, find the horizontal offset necessary to highlight that position
getColumn = (y, x) ->
  row = document.getElementById "row-#{y}"
  if row?
    +row.dataset.offset + 2 * x
  else
    -1

highlightIndex = (x, y, color) ->
  col = getColumn y, x
  element = (highlights[color] ?= document.createElement 'div')
  element.classList.add 'highlight'
  element.style = "--highlight-color: #{color};"
  element.style.left = "#{col}ch"
  elements.debugProgram.insertBefore element, document.getElementById "row-#{y}"

hideUrl = ->
  elements.urlOutBox.className = 'content-hidden'
  elements.urlButton.textContent = 'Generate URL'
  elements.urlButton.onclick = generateURL
elements.program.addEventListener 'input', hideUrl, passive: true
elements.includeInput.addEventListener 'change', hideUrl, passive: true

# Allow the header itself, or noninteractable children (e.g. the select icon)
isHeader = (target) ->
  target is elements.debugHeader or (target.tagName isnt 'BUTTON' and elements.debugHeader.contains target)

# Adapted from https://www.w3schools.com/howto/howto_js_draggable.asp
pos3 = 0
pos4 = 0
elements.debugHeader.onmousedown = (e) ->
  if not isHeader e.target
    return
  e.preventDefault()
  pos3 = e.clientX
  pos4 = e.clientY
  document.onmousemove = (e) ->
    e.preventDefault()
    pos1 = pos3 - e.clientX
    pos2 = pos4 - e.clientY
    pos3 = e.clientX
    pos4 = e.clientY
    elements.debugInfo.style.top = "#{elements.debugInfo.offsetTop - pos2}px"
    elements.debugInfo.style.left = "#{elements.debugInfo.offsetLeft - pos1}px"
  document.onmouseup = ->
    @onmousemove = null
    @onmouseup = null
elements.debugHeader.ontouchstart = (e) ->
  if not isHeader e.target
    return
  e.preventDefault()
  pos3 = e.touches[0].clientX
  pos4 = e.touches[0].clientY
  document.ontouchmove = (e) ->
    e.preventDefault()
    pos1 = pos3 - e.touches[0].clientX
    pos2 = pos4 - e.touches[0].clientY
    pos3 = e.touches[0].clientX
    pos4 = e.touches[0].clientY
    elements.debugInfo.style.top = "#{elements.debugInfo.offsetTop - pos2}px"
    elements.debugInfo.style.left = "#{elements.debugInfo.offsetLeft - pos1}px"
  document.ontouchend = document.ontouchcancel = ->
    @ontouchmove = null
    @ontouchcancel = null
    @ontouchend = null

width = elements.runStop.offsetWidth
remSize = parseFloat getComputedStyle(document.body).fontSize
elements.runStop.textContent = 'Run!'
setTimeout -> elements.runStop.style.width = "#{(0.49 + Math.max width, elements.runStop.offsetWidth) / remSize}rem"
elements.runStop.onclick = interpretProgram
elements.urlButton.onclick = generateURL

if location.hash.length > 1
  elements.program.value = decodeURIComponent location.hash.slice 1

params = new URLSearchParams location.search
inputParam = params.get 'i'
if inputParam?
  elements.stdin.value = inputParam
  elements.includeInput.checked = true
