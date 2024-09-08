#!/bin/bash

set -euo pipefail

folder=$(dirname "$0")
root=$(realpath "${folder}/../../..")

run_qdeql () {
    if [ "$#" -gt 1 ]
    then
        printf '%s\0%s' "$(cat "$1")" "$2" | $TRILANGLE -af "${root}/qdeql/interpreter.trg"
    else
        $TRILANGLE "${root}/qdeql/interpreter.trg" <"$1"
    fi
}

# hello world
test 'hello world' = "$(run_qdeql "${folder}/hello.qd")"

# cat
# Qdeql is limited to bytes, so this cat can't be the same Unicode cat
text='
    /\_/\           ___
   = 0_0 =_______    \ \
    __^      __(  \.__) )
(@)<_____>__(_____)____/'

output=$(run_qdeql "${folder}/cat.qd" "$text")
test "$text" = "${output//$'\r'/}"

# truth machine
test 0 = "$(run_qdeql "${folder}/tm.qd" 0)"
test 1111111111 = "$(run_qdeql "${folder}/tm.qd" 1 | head -c 10)"
