#!/bin/bash

set -eu

invalid_flags=(
    # Not actually a flag
    -
    # various mode flags together
    -Ae -ce -De -AD -Dc -Ac
    # --assume-ascii without actually executing
    -ae -aD
    # --show-stack without --debug
    -s
    # --hide-nops without --disassemble
    -n
)

for flag in "${invalid_flags[@]}"
do
    result=0
    errors=$($TRILANGLE "$flag" 2>&1 1<&-) || result=$?
    test 64 = $result
    test -n "$errors"
done
