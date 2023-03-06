# Turing Completeness

One way to prove a language is Turing-complete is to prove that it can interpret another language that is known to be Turing-complete. This document will outline the implementation of Brainfuck (except the input instruction `,`) in Trilangle.

Though this is possible without the threading operators `{}`, not using them imparts severe restrictions on the Brainfuck program (most notably, that the number of `+` and `-` instructions executed may not exceed 255).

## Broad overview

The program will consist of two long-running threads, henceforth called the *code thread* and the *tape thread*. The *code thread* will start with a program taken from stdin, and keep the IP on top of the stack. The *tape thread* will start with 256 zeroes, and keep the MP on top of the stack.

Each tick of the Brainfuck program (many ticks of the interpreter), the following will happen:

1. The tape thread will read the value pointed at by the MP, and send a copy of it to the code thread.
2. Upon receiving the value, the code thread will fetch the next instruction. If that instruction is `[`, `]`, or `.`, the code thread will execute it without consulting the tape thread. If that instruction is `+`, `-`, `<`, or `>`, the code thread will send that instruction to the tape thread.
3. When the tape head receives an instruction, it will perform that action, updating either the MP or the tape itself.
4. Repeat this cycle until the end of the program is reached.

## Thread synchronization

Since the thread join operation goes by order of arrival, care must be taken to ensure that joins always happen in the same order.

There are two potential approaches here: busy loops and signal threads.

### Busy loops

Since mutating the tape is expensive (see below), the code thread will likely get ahead of the tape thread. By having the code thread count to a sufficiently large number before sending instructions to the tape thread, it is guaranteed that the instruction will join after the tape thread, and the new memory value will join before the code thread.

### Signal threads

When a thread hits a join with `0` at the top of its stack, its stack is entirely discarded, so order of arrival does not matter. Each thread join mentioned above may be implemented with a pair of joins, such as in the following assembly:

```
; Code thread
; ...
c_0: TJN ; wait for signal
c_1: TJN ; read value from tape thread, which will always get here first
; ...

; Signal thread
s_0: PSI #0 ; prevent this stack from affecting the code thread
s_1: JMP c_0

; Communication thread
v_0: PSI #1 ; preserve the top value on this stack
v_1: JMP c_1

; Tape thread
; ...
t_0: TSP v_0 ; create the thread that communicates the needed value
t_1: TSP s_0 ; create the signal thread
; ...
```

## Mutating the tape

Most of the operations mentioned above are fairly straightforward. However, indexing is read-only, which poses a challenge for `+` and `-` instructions. Instead, these instructions will need to copy the entire tape, with one value different.

> Note: this section may no longer be 100% accurate, as it's copied almost verbatim from a previous revision that did not have threading. The approach described still works, even if it's not the best way to do it.

It's not necessarily possible to keep working variables, as the copy operation must carry the MP with it.

The key to being able to copy the memory tape without any working variables is to limit Brainfuck to use no more than 14 bits per cell. The 24-bit value will look something like this:

| 8 bits | 1 bit | 1 bit | 14 bits |
|--|--|--|--|
| Distance to the cell to be changed | `+` vs `-` | Is this the end of the tape? | Copy of the low 14 bits off the previous tape |

When copying the very end of the memory tape, the high byte will be set to `255 - tape head`, and for each cell afterwards it will be one less than the value before it. Once the high byte is 0, the next bit will be used to determine whether to increment or decrement the low 14 bits.

After incrementing or decrementing the appropriate cell, the interpreter copies the remaining memory verbatim until the tenth bit is set, which indicates it's reached the end of the tape.
