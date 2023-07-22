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
  stdin = => if inputIndex >= stdinBuffer.length then null else stdinBuffer.at inputIndex++
  stdout = (char) => postMessage [1, char]
  stderr = (char) => postMessage [2, char]
  FS.init stdin, stdout, stderr
Module['onRuntimeInitialized'] = halfReady
Module['noExitRuntime'] = true

callInterpreter = (warnings, disassemble, expand) => =>
  inputIndex = 0
  try
    Module['ccall'] 'wasm_entrypoint',
      null,
      ['string', 'number', 'number', 'number'],
      [programText, warnings, disassemble, expand]
  catch e
    if e? and not (e instanceof ExitStatus)
      postMessage [2, e.toString()]

signals.set 'interpretProgram', callInterpreter 1, 0, 0
signals.set 'disassembleProgram', callInterpreter 0, 1, 0
signals.set 'expandInput', callInterpreter 0, 0, 1

@onmessage = (event) ->
  stdinBuffer = encoder.encode event.data[2]
  programText = event.data[1]
  halfReady event.data[0]
