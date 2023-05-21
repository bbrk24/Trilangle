#pragma once

#include <cstdint>

#define DIRECTION_INSENSITIVE_OPS \
    NOP = '.',      /* no-op */ \
        ADD = '+',  /* add */ \
        SUB = '-',  /* subtract */ \
        MUL = '*',  /* multiply */ \
        DIV = ':',  /* divide */ \
        MOD = '%',  /* modulo */ \
        PSI = '\'', /* push integer */ \
        PSC = '"',  /* push character */ \
        POP = ',',  /* pop */ \
        EXT = '@',  /* exit */ \
        INC = ')',  /* increment */ \
        DEC = '(',  /* decrement */ \
        AND = '&',  /* bitwise and */ \
        IOR = 'r',  /* bitwise or */ \
        XOR = 'x',  /* bitwise x-or */ \
        NOT = '~',  /* bitwise not */ \
        GTC = 'i',  /* getchar */ \
        PTC = 'o',  /* putchar */ \
        GTI = '?',  /* get integer */ \
        PTI = '!',  /* put integer */ \
        IDX = 'j',  /* index */ \
        DUP = '2',  /* duplicate */ \
        DP2 = 'z',  /* 2-dupe */ \
        RND = '$',  /* random */ \
        EXP = 'e',  /* exponential */ \
        SWP = 'S',  /* swap */ \
        GTM = 'T',  /* get time */ \
        GDT = 'D'   /* get date */

// The valid source tokens.
enum opcode : int32_t {
    BNG_NW = '^',     // branch if negative, pointing northwest
    BNG_NE = '7',     // branch if negative, pointing northeast
    BNG_E = '>',      // branch if negative, pointing east
    BNG_SE = 'v',     // branch if negative, pointing southeast
    BNG_SW = 'L',     // branch if negative, pointing southwest
    BNG_W = '<',      // branch if negative, pointing west
    MIR_EW = '_',     // mirror east/west
    MIR_NS = '|',     // mirror north/south
    MIR_NESW = '/',   // mirror northeast/southwest
    MIR_NWSE = '\\',  // mirror northwest/southeast
    THR_W = '{',      // west thread operator
    THR_E = '}',      // east thread operator
    SKP = '#',        // skip
    DIRECTION_INSENSITIVE_OPS
};
