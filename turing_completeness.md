# Turing Completeness

One way to prove a language is Turing-complete is to prove that it can interpret another language that is known to be Turing-complete. This document will outline the implementation of Brainfuck (except the input instruction `,`) in Trilangle.

## Program setup

The interpreter will start by reading from STDIN, keeping track of how many characters it has read so far. This input will be the Brainfuck program to run.

After it reaches EOF, the interpreter will initialize an empty tape of 256 memory cells, keeping the program length on top. The stack will look something like this:

    [Brainfuck code] [256 zeroes] [length of code]

I have already fully implemented this part of the program. In the code below, this initialization will take place and then the program immediately terminates:

```
            '
           0 .
          < _ .
         \ . > i
        8 S ) / e
       ' \ . / ( .
      \ , < . . . .
     # r * e 8 ' / .
    & * ( 2 e 8 ' 2 <
   . . . . . . . L ( .
  e 8 ' S 0 ' , 7 . . -
 . . . . . . @ , / . . ^
```

## Program execution

The value on top of the stack, initially the length of the program, will be used as a sort of instruction pointer: by adding 256 (the size of the stack), it can be used as the operand for a `j` operation to fetch the current Brainfuck instruction.

You may have noticed by now that I have not set up the tape head. Additionally, indexing is read-only, and it's only possible to write to the top two values on the stack. To account for this, the value on top of the stack will store three 1-byte values rather than a single 3-byte value. During general execution, the memory will look something like this:

    [Brainfuck code] [n*256 garbage values] [memory tape] [bytefield: garbage count; tape head; IP]

The garbage count and tape head location are both initialized to 0.

The index of the current instruction can be computed as `(garbage count + 1) * 256 + IP + 1`. After fetching the instruction, it should be one of eight values:

- `.`: Read from the active memory tape and output the value in the current cell.
- `<`: Decrement the tape head.
- `>`: Increment the tape head.
- `[`: Read from the active memory tape. If the value in the current cell is 0, decrement the IP until it points to a `]`.
- `]`: Increment the IP until it points to a `[`.
- `+` or `-`: (see below)
- NULL: Terminate program.

When `+` or `-` is read, the program will copy the entire memory tape, but with one value different. As it's not possible to have any working variables (remember, it must carry the GC/TH/IP with it), I had to get creative about how the copy operation works.

## Copying the memory tape

The key to being able to copy the memory tape without any working variables is to limit Brainfuck to use no more than 14 bits per cell. The 24-bit value will look something like this:

| 8 bits | 1 bit | 1 bit | 14 bits |
|--|--|--|--|
| Distance to the cell to be changed | `+` vs `-` | Is this the end of the tape? | Copy of the low 14 bits off the previous tape |

When copying the very end of the memory tape, the high byte will be set to `255 - tape head`, and for each cell afterwards it will be one less than the value before it. Once the high byte is 0, the next bit will be used to determine whether to increment or decrement the low 14 bits.

After incrementing or decrementing the appropriate cell, the interpreter copies the remaining memory verbatim until the tenth bit is set, which indicates it's reached the end of the tape. Finally, the garbage counter is incremented, and then the interpreter goes back to continue the fetch-decode-execute cycle.

### Accounting for the layout of the tape

When implementing the `.` and `[` instructions, only the low 14 bits of the cell should be used. This can be done relatively simply using this instruction sequence (read left to right):

```
' 4 e ( e ( &
```
