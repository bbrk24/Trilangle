inputIndex = 0
ready = false
stdinBuffer = null
programText = ''
funcName = null
encoder = new TextEncoder
signals = new Map

halfReady = (arg) ->
  funcName = arg ? funcName
  unless ready
    ready = true
    return
  signal = signals.get funcName
  if signal?
    signal()
  else
    console.error "Unrecognized signal #{funcName}"
    postMessage [0, null]

Module['preInit'] = ->
  stdin = -> if inputIndex >= stdinBuffer.length then null else stdinBuffer.at inputIndex++
  stdout = (char) -> postMessage [1, char]
  stderr = (char) -> postMessage [2, char]
  FS.init stdin, stdout, stderr
Module['onRuntimeInitialized'] = halfReady
Module['noExitRuntime'] = true

callInterpreter = (warnings, disassemble, expand) -> ->
  inputIndex = 0
  try
    await Module['ccall'] 'wasm_entrypoint',
      null,
      ['string', 'number', 'number', 'number'],
      [programText, warnings, disassemble, expand],
      async: true
  catch e
    if e? and not (e instanceof ExitStatus)
      postMessage [2, e.toString()]
  postMessage [0, null]

signals.set 'interpretProgram', callInterpreter 0, 0, 0
signals.set 'disassembleProgram', callInterpreter 1, 0, 0
signals.set 'expandInput', callInterpreter 0, 1, 0
signals.set 'debugProgram', callInterpreter 0, 0, 1
signals.set 'step', => @['__trilangle_resolve']()

@onmessage = (event) ->
  if event.data.length > 1
    stdinBuffer = encoder.encode event.data[2]
    programText = event.data[1]
  halfReady event.data[0]
