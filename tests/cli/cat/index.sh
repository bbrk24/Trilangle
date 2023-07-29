#!/bin/bash

set -e

text='
    /\_/\           ___
   = ಠ_ಠ =_______    \ \
    __^      __(  \.__) )
(@)<_____>__(_____)____/'

folder="$(dirname "$0")"

output="$(trilangle "${folder}/cat.trg" <<<"$text")"

test "$text" = "$output"
