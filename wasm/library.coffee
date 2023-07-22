mergeInto LibraryManager.library, {
    send_thread_count: (thread_count) ->
        postMessage [3, thread_count]
    ,
    send_debug_info: (thread_number, stack, stack_depth, x, y, instruction) ->
        postMessage [3, [thread_number, {
            x,
            y,
            instruction: String.fromCodePoint(instruction),
            stack: Array::map.call HEAP32.slice(stack >> 2, (stack >> 2) + stack_depth), (x) => x << 8 >> 8
        }]]
}