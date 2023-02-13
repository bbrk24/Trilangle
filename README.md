# Triagony

NOTE: "Triagony" is a working title and likely will not be its final name.

Triagony is a 2-D, stack-based programming language inspired by [Hexagony].

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

Triagony ditches the hexagonal memory layout in favor of a stack. Technically, it's not a pure stack as it is possible to index into it, but the majority of operations treat it as such.

Each item on the stack is a 24-bit signed integer.

## Instructions

In general, the instruction set is inspired by but not necessarily the same as Hexagony.

### Control flow splits

In this grid, it is no longer possible to run off a corner in the direction of that corner. To make up for this, there are now six characters that act as splits in program flow, rather than two. These are:

- Northeast `7`
- East `>`
- Southeast `v`
- Southwest `L`
- West `<`
- Northwest `^`

When the IP hits the "point" of any of these instructions, it deflects to the left if the value on top of the stack is negative, and it deflects to the right if the value on top of the stack is positive or zero. Besides the treatment of zero, `>` and `<` act the same as in Hexagony, and the other four are rotations thereof.

### Mirrors

Triagony has the same four mirrors as Hexagony: `|`, `_`, `/`, and `\`.

### Other control flow

There are two other control flow instructions: `#` causes the interpreter to ignore the next character the IP hits, and `@` terminates the program.

There is also a NOP, `.`.

### Numeric instructions

Triagony has twelve numeric instructions that operate purely on the stack. In no particular order, these are:

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
- `j` (IDX): pop the value on top of the stack and use it as an index for the rest of the stack. If the value popped is _i_, this instruction pushes a copy of the _i_-th value from the top of the stack (zero-indexed).
- `S` (SWP): swap the top two values of the stack.

### I/O instructions

Triagony has five instructions for I/O: two for console input, two for console output, and one for randomness.

- `i` (GTC): read a single UTF-8 character from STDIN and push it to the stack. Pushes -1 on EOF.
- `?` (GTI): read an integer from STDIN and push it to the stack. Pushes -1 on EOF.
- `o` (PTC): write the top of the stack as a single character to STDOUT. This currently does not work for multi-byte characters on Windows due to console limitations.
- `!` (PTI): write the value of the top of the stack as a decimal number to STDOUT.
- `$` (RND): push a random 24-bit integer to the stack.

Note that the input functions push to the stack, but the output functions do not pop from it.

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

### AAAAAAAAAA

```
  "
 A ,
o . .
```

[Hexagony]: https://github.com/m-ender/hexagony