# Trilangle

Trilangle is a 2-D, stack-based programming language inspired by [Hexagony].

## Program layout

Like Hexagony, the characters are layed out in a hexagonal grid. Unlike Hexagony, however, the bounding box of the program as a whole is triangular, not hexagonal. This means that a program grid might look like

```
    .
   . .
  . . .
 . . . .
. . . . .
```

When the IP runs off the edge of the board, it starts again on the row or diagonal one spot to its left. The IP starts at the north corner heading southwest, so in a 10-cell (4×4) grid, it hits the cells in this order:

```
   0
  1 4
 2 5 7
3 6 8 9
```

and once it walks off the eastern corner, it starts over again at `0`.

The interpreter does not require the spaces and line breaks, but it ignores them if they are there. So, that program is the same as a one-line program that reads

```
0142573689
```

Additionally, trailing `.`s on the end of the last line may be omitted.

## Memory Layout

Trilangle ditches the hexagonal memory layout in favor of a stack. Technically, it's not a pure stack as it is possible to index into it, but the majority of operations treat it as such.

Each item on the stack is a 24-bit signed integer.

## Instructions

### Control flow splits

In this grid, it is no longer possible to run off a corner in the direction of that corner. To make up for this, there are now six characters that act as splits in program flow, rather than two. These are:

- Northeast `7`
- East `>`
- Southeast `v`
- Southwest `L`
- West `<`
- Northwest `^`

When the IP hits the "point" of any of these instructions, it deflects to the left if the value on top of the stack is negative, and it deflects to the right if the value on top of the stack is positive or zero. Besides the treatment of zero, `>` and `<` act the same as in Hexagony, and the other four are rotations thereof.

For completeness, here's a table of them. The first column is the initial IP direction, and the other columns are the direction of the IP after the branch.

| | `7` | `>` | `v` | `L` | `<` | `^` |
|--|--|--|--|--|--|--|
| NE | SW | E | SW | E/NW | SW | NW |
| E | NE | W | SE | W | SE/NE | W |
| SE | NW | E | NW | SW | NW | SW/E |
| SW | W/SE | NE | SE | NE | W | NE |
| W | E | NW/SW | E | SW | E | NW |
| NW | NE | SE | NE/W | SE | W | SE |

### Mirrors

Trilangle has the same four mirrors as Hexagony: `|`, `_`, `/`, and `\`. They operate as follows:

<!-- Since | is the column separator, I have to escape it. But I can't use an escape sequence inside backticks, so I have to use the HTML <code> tag instead. -->

| | <code>\|</code> | `_` | `/` | `\` |
|--|--|--|--|--|
| NE | NW | SE | NE | W |
| E | W | E | NW | SW |
| SE | SW | NE | W | SE |
| SW | SE | NW | SW | E |
| W | E | W | SE | NE |
| NW | NE | SW | E | NW |

### Other control flow

There are two other control flow instructions: `#` causes the interpreter to ignore the next character the IP hits, and `@` terminates the program.

There is also a NOP, `.`.

### Numeric instructions

Trilangle has twelve numeric instructions that operate purely on the stack. In no particular order, these are:

- `+` (ADD): pop two numbers from the stack and push their sum.
- `-` (SUB): pop two numbers from the stack and push their difference\*.
- `*` (MUL): pop two numbers from the stack and push their product.
- `:` (DIV): pop two numbers from the stack, perform integer division and push the quotient\*.
- `%` (MOD): pop two numbers from the stack, perform integer division and push the remainder\*.
- `(` (DEC): decrease the value on top of the stack by 1.
- `)` (INC): increase the value on top of the stack by 1.
- `e` (EXP): pop a number _x_ from the stack and push 2<sup>_x_</sup>.
- `&` (AND): pop two numbers from the stack and push their bitwise intersection.
- `r` (IOR): pop two numbers from the stack and push their bitwise union.
- `x` (XOR): pop two numbers from the stack and push their bitwise difference.
- `~` (NOT): pop a number from the stack and push its bitwise complement.

\* SUB, DIV, and MOD use the top of the stack as the right operand, and the second value from the top as the left operand.

### Stack-oriented instructions

There are a few instructions that operate on the stack directly.

- `"` (PSC): push the Unicode value of the next character hit by the IP.
- `'` (PSI): push the integer value of the next character hit by the IP. This currently works by subtracting the value of '0' from the Unicode value of the character.
- `,` (POP): pop and discard the value on top of the stack.
- `2` (DUP): copy and push the top value on the stack.
- `j` (IDX): pop a value _i_ from top of the stack and push the _i_-th value of the remainder of the stack (0-based).
- `S` (SWP): swap the top two values of the stack.

### I/O instructions

Trilangle has five instructions for I/O: two for console input, two for console output, and one for randomness.

- `i` (GTC): read a single UTF-8 character from STDIN and push it to the stack. Pushes -1 on EOF.
- `?` (GTI): read an integer from STDIN and push it to the stack. Pushes -1 on EOF.
- `o` (PTC): write the top of the stack as a single character to STDOUT. This currently does not work for multi-byte characters on Windows due to console limitations.
- `!` (PTI): write the value of the top of the stack as a decimal number to STDOUT.
- `$` (RND): push a random 24-bit integer to the stack.

Note that the input functions push to the stack, but the output functions do not pop from it.

### Threading

`{` and `}` can be used for thread manipulation. They behave nearly identically (with `}` behaving as the mirror image of `{`), so only one will be described in detail.

When `{` is approached from the west, the interpreter splits into two "threads": one going northeast, and one going southeast. The threads receive a deep copy of the stack: that is, they start with the same contents, but mutating one does not affect the other.

When `{` is approached from the east, the thread is removed. If no more threads remain, the program is terminated. (Note that `@` can terminate the whole program without waiting for other threads.)

When `{` is approached from the northwest or southwest, the interpreter passes right over it, as with `.`.

When `{` is approached from the northeast or southeast, that interpreter thread pauses until a second one reaches the same location from the northeast or southeast. When two threads are waiting at the same place, they are merged and the resulting thread continues to the west. The top value on each stack indicates how many values to move into the resulting stack, for example:

| First thread | Second thread | Result |
|--|--|--|
| 3 | 1 | f |
| c | f | c |
| b | e | b |
| a | d | a |

## Interpreter flags

When run with the `-d` flag, the interpreter enters "debug mode." Before executing each instruction, it prints the location of the IP and the opcode that it is pointing at, and it waits for you to press enter before continuing. If the `-s` flag is also set, the interpreter will print the stack contents as well as the IP.

If the `-w` flag is set, warnings will be printed to STDERR for undefined behavior.

If the `-f` flag is set, programs terminate once STDOUT cannot be written to. This allows, for example, piping the output of the program into `head` to terminate early.

## The disassembler

As you may have noticed above, every instruction has a three-letter name. When the interpreter is given the flag `-D`, rather than interpreting code, it converts it to a pseudo-assembly program.

In additon to actual NOPs, it reports mirrors, skips, and some branch instructions as NOPS. Since this results in an excess of NOPs in the output, you may additionally pass the `-n` flag to hide NOPs.

When laying out the program linearly, the disassembler represents branch instructions as `BNG` (branch if negative) statements, and loops as `JMP` (jump) statements. The threading operations are represented as `TSP` (thread spawn), `TJN` (thread join), and `TKL` (thread kill).

For example, when passing [the cat program below](#cat) with the flags `-Dn`, the output is as follows:

```
4:      GTC
5:      BNG 12
7:      PTC
9:      POP
11:     JMP 2
12:     EXT
```

## Sample Programs

Here are some simple programs I've written.

### cat

Simply outputs its input until EOF is read.

```
    #
   @ .
  . L \
 v i o .
. _ \ , .
```

### Hello, World!

Everyone's favorite program. Does some math on the value of `!` to get the space and newline characters.

```
        "
       H o
      o " !
     " o ( o
    e o o o l
   o " " " o "
  " , W r " ! 3
 l o o o d o : o
o " " " o ' ( @ .
```

### Count to 100

Pushes the same zero twice, from different directions.

```
      '
     0 .
    v j .
   . ! " /
  @ . ) e .
 , > - . / .
. _ . . ' . .
```

### Truth Machine

Reads a single number in. If the number is 0, it prints back a single zero and exits. If the number is 1, it prints infinitely many ones back.

```
    ?
   ! <
  ( @ 7
 \ < . #
^ ) / . .
```

### Prime Test

Reads in an integer, and prints '1' if it's prime.

```
        '
       2 .
      ? . .
     < _ . @
    j . 2 ' 2
   , < | > ( %
  ! . \ S ) S ,
 , ) S < . . . .
. @ > - . . . . .
```

### GCD

Reads two numbers in and prints their GCD. Doesn't like zero as an input.

```
     ?
    ? ,
   < ! .
  j . 1 '
 > ( | # %
. @ \ S ) <

```

### AAAAAAAAAA

```
  "
 A ,
o . .
```

## Proof of Turing-completeness

See [turing_completeness.md].

## Compiling the interpreter

### Compiling natively

The specific compiler used shouldn't matter. I've been using MSVC 14.34 (Visual Studio 2022 version 17.4), but it should be compatible with clang and GCC, as well as some earlier versions of MSVC (Visual Studio 2017 or later). Incompatibility with these compilers is considered a bug, and any issues should be reported on [the issues page][issues].

C++14 (`201304L`) or later is required to compile this project. Certain features from newer versions will be used if they are available, which may affect the performance of the compiled binary. These features include, but are not limited to:

- `noexcept` in function types (C++17, `201510L`)
- `constexpr` lambdas (C++17, `201603L`)
- the `[[unlikely]]` attribute (C++20, `201803L`)

As such, it is recommended to use C++20 or later if performance is a concern. (If it is, why? How are you possibly using this language in a performance-sensitive environment?)

### Compiling for the web

The wasm directory contains most of the wrapper/glue code necessary to run the interpreter in a browser. Emscripten 3.1.21 or later and Node 14 or later are required.

Run the shell script `build_wasm.sh` to compile the project to webassembly and to generate the JS glue code. Then, you can run `npm start` to host it locally (at http://localhost:3000), or `npm run deploy` to push it to Github pages.

[Hexagony]: https://github.com/m-ender/hexagony
[turing_completeness.md]: turing_completeness.md
[issues]: https://github.com/bbrk24/Trilangle/issues
