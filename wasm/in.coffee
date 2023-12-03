'use strict'

worker = null
stdoutBuffer = []
stderrBuffer = []
threads = []
decoder = new TextDecoder
resolve = null
delay = 500
interval = -1
colors = new Colors

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
  programText = elements.program.value.replace /\n| /gu, ''
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
          colors.hideHighlights()
        else
          threads[content[0]] = content[1]
          try
            if threadCount is threads.reduce (x) -> x + 1, 0
              renderThreads()
            else
              step()
          catch e
            if e instanceof Error
              elements.stderr.innerText += e.message + '\n'
            else
              elements.stderr.innerText += "#{e}\n"
            wasmCancel()
      else console.error "Unrecognized destination #{event.data[0]}", content
    return
  worker.postMessage [name, elements.program.value, elements.stdin.value]
  
  promise

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
  elements.colorPicker.hidden = true

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

  colors.destroyHighlights()
  elements.debugInfo.hidden = true
  elements.debugProgram.innerHTML = ''
  elements.debugProgram.hidden = true
  elements.program.hidden = false
  pause()

renderThreads = ->
  elements.threads.innerHTML = ''
  threads.forEach (thread, idx) ->
    row = document.createElement 'tr'
    highlightDiv = colors.getHighlightDiv(idx) ? throw new TypeError(
      "You haven't selected enough colors for that many threads!"
    )
    color = highlightDiv.style.getPropertyValue '--highlight-color'
    row.style.backgroundColor = color
    highlightIndex thread.x, thread.y, highlightDiv
    threadIdEl = document.createElement 'td'
    threadIdEl.textContent = String idx
    row.appendChild threadIdEl
    threadContentsEl = document.createElement 'td'
    threadContentsEl.textContent = thread.stack.join ',\u2009'
    row.appendChild threadContentsEl
    elements.threads.appendChild row

changeHighlightColor = (before, after) ->
  threadNum = colors.replaceColor before, after
  return true unless threadNum?
  return false if threadNum < 0
  iter = document.evaluate "//td[text() = #{threadNum}]/..",
    document,
    null,
    XPathResult.ANY_TYPE,
    null
  node = iter.iterateNext()
  if node?
    node.style.backgroundColor = after
  true

# Given y,x, find the horizontal offset necessary to highlight that position
getColumn = (y, x) ->
  row = document.getElementById "row-#{y}"
  if row?
    +row.dataset.offset + 2 * x
  else
    -1

highlightIndex = (x, y, div) ->
  col = getColumn y, x
  div.style.left = "#{col}ch"
  elements.debugProgram.insertBefore div, document.getElementById "row-#{y}"

hideUrl = ->
  elements.urlOutBox.className = 'content-hidden'
  elements.urlButton.textContent = 'Generate URL'
  elements.urlButton.onclick = generateURL
elements.program.addEventListener 'input', hideUrl, passive: true
elements.includeInput.addEventListener 'change', hideUrl, passive: true
elements.stdin.addEventListener 'change', ->
  hideUrl() if elements.includeInput.checked
, passive: true

toHex = (rgba) ->
  return rgba if /^#[0-9a-d]{6}$/i.test rgba
  # Adapted from https://stackoverflow.com/a/3627747/6253337
  "\##{
    rgba.match /^rgba?\((\d+),\s*(\d+),\s*(\d+)(?:,\s*(\d+\.{0,1}\d*))?\)$/
      .slice 1
      .map (n, i) ->
        (if i is 3 then Math.round(255 * parseFloat n) else parseInt n, 10)
          .toString 16
          .padStart 2, '0'
          .replace 'NaN', ''
      .join ''
  }"

createColorDiv = (color, i) ->
  input = document.createElement 'input'
  input.type = 'color'
  input.value = toHex color
  input.onchange = ->
    unless changeHighlightColor i, @value
      @value = toHex color
      elements.stderr.innerText += "You can't have two threads of the same color!\n"

  removeButton = document.createElement 'button'
  div = document.createElement 'div'
  div.appendChild input
  div.appendChild removeButton
  
  removeButton.type = 'button'
  removeButton.title = 'remove color'
  removeButton.onclick = ->
    if colors.removeColor i
      updateColorPicker()
      renderThreads()
  removeButton.appendChild document.createTextNode 'x'
  div

updateColorPicker = ->
  elements.colorPicker.innerHTML = ''
  colors.allColors().forEach (color, i) ->
    div = createColorDiv color, i
    elements.colorPicker.appendChild div
  
  if not elements.addColorButton?
    button = document.createElement 'button'
    button.id = 'add-color-button'
    button.type = 'button'
    button.title = 'add color'
    button.appendChild document.createTextNode '+'
    button.onclick = ->
      loop
        red = Math.round 255 * Math.random()
        green = Math.round 255 * Math.random()
        blue = Math.round 255 * Math.random()
        color = "rgb(#{red}, #{green}, #{blue})"
        break if colors.addColor color
      elements.colorPicker.insertBefore createColorDiv(color, colors.count() - 1), @
    elements.addColorButton = button
  elements.colorPicker.appendChild elements.addColorButton

@toggleColorPicker = ->
  if elements.colorPicker.hidden
    updateColorPicker()
    elements.colorPicker.hidden = false
  else
    elements.colorPicker.hidden = true

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
