# This file is run by emscripten rather than being copied into worker.js. As such, any global variable declarations will
# be lost, and the closure compiler will complain about undeclared variables (even if the JS runtime is okay with it).
# Therefore, global variables are instead properties on globalThis. To avoid potential name conflicts with anything
# emscripten might do (I have no idea what, if any, properties on globalThis it uses), I'm naming them to start with
# "__trilangle_". Things that are also accessed in worker.coffee arae represented in quotes in case the minifier mangles
# them separately.
mergeInto LibraryManager.library,
  send_thread_count: (thread_count) =>
    @__trilangle_threadCount = thread_count
    postMessage [3, thread_count],
  send_debug_info: (thread_number, stack, stack_depth, y, x, instruction) =>
    Asyncify.handleAsync =>
      postMessage [3, [thread_number, {
        x,
        y,
        instruction: String.fromCodePoint(instruction),
        stack: Array::map.call HEAP32.slice(stack >> 2, (stack >> 2) + stack_depth), (x) => x << 8 >> 8
      }]]
      if thread_number is @__trilangle_threadCount
        new Promise (r) =>
          @['__trilangle_resolve'] = r
          undefined
      else
        Promise.resolve()
