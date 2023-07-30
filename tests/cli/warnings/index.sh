#!/bin/bash

folder="$(dirname "$0")"

# Without the braces, the &> doesn't silence the floating-point exception
{ trilangle -w "${folder}/worse.trg"; } &>/dev/null
result=$?
set -e
test $result -ne 0

errors="$(trilangle -w "${folder}/bad.trg" 2>&1)"

test -n "$errors"
