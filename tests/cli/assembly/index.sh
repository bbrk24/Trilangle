#!/bin/bash

set -eu

# Adapted from https://cedwards.xyz/defer-for-shell/
DEFER=''
defer () {
    DEFER="$*; $DEFER"
    # shellcheck disable=SC2064
    trap "{ $DEFER}" EXIT
}

text='
    /\_/\           ___
   = ಠ_ಠ =_______    \ \
    __^      __(  \.__) )
(@)<_____>__(_____)____/'

folder=$(dirname "$0")

errors=$(mktemp)
defer rm "$errors"

output=$($TRILANGLE -fwA "${folder}/cat.txt" <<<"$text" 2>"$errors")
test ! -s "$errors"
test "$text" = "${output//$'\r'/}"
