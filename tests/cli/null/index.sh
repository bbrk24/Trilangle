#!/bin/bash

set -euo pipefail

text='
    /\_/\           ___
   = ಠ_ಠ =_______    \ \
    __^      __(  \.__) )
(@)<_____>__(_____)____/'

run_with_z () {
    printf '%s\0%s' "$1" "$2" | $TRILANGLE -z
}

one=$(run_with_z '?!@' 1)
z=$(run_with_z 'io@' z)
output=$(run_with_z '<>i,@##o' "$text")

test 1 = "$one"
test z = "$z"
test "$text" = "${output//$'\r'/}"
