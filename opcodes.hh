#pragma once

#include <cstdint>

enum opcodes : int32_t {
    NOP = '.',
    ADD = '+',
    SUB = '-',
    MUL = '*',
    DIV = 'd',
    MOD = '%',
    BSN_NW = '^',
    BSN_NE = '7',
    BSN_E = '>',
    BSN_SE = 'v',
    BSN_SW = 'L',
    BSN_W = '<',
    MIR_EW = '_',
    MIR_NS = '|',
    MIR_NESW = '/',
    MIR_NWSE = '\\',
    PSH = '\'',
    POP = ',',
    EXT = '@',
    INC = '(',
    DEC = ')',
    AND = '&',
    IOR = 'r',
    XOR = 'x',
    NOT = '~',
    GTC = 'i',
    PTC = 'o',
    GTI = '?',
    PTI = '!',
    SKP = '#',
    LEA = 'j',
};
