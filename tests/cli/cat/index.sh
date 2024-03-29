#!/bin/bash

set -eu

text='
    /\_/\           ___
   = ಠ_ಠ =_______    \ \
    __^      __(  \.__) )
(@)<_____>__(_____)____/'

folder=$(dirname "$0")

output=$($TRILANGLE "${folder}/cat.trg" <<<"$text")
# Okay so on Windows the newline characters are different. Isn't this fun?
test "$text" = "${output//$'\r'/}"

x=$((0x912345))
test $x = "$($TRILANGLE "${folder}/uint.trg" <<<$x)"
