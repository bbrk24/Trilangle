#!/bin/bash

set -euo pipefail

folder=$(dirname "$0")

trilangle -c "${folder}/cat.trg" | $CC -o ./trgcat.out -xc -

text='
    /\_/\           ___
   = O_O =_______    \ \
    __^      __(  \.__) )
(@)<_____>__(_____)____/'
output=$(./trgcat.out <<<"$text")

test "$text" = "$output"
