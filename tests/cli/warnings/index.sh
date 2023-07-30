#!/bin/bash

folder="$(dirname "$0")"

# Without the braces, the &> doesn't silence the floating-point exception
# Without the subshell, exec kills the terminal
{ errors1="$( (exec trilangle -w "${folder}/worse.trg" 2>&1 1<&-) )"; } &>/dev/null
result=$?
set -e
test $result -ne 0

errors2="$(trilangle -w "${folder}/bad.trg" 2>&1)"

test -n "$errors1"
test -n "$errors2"
