#!/bin/bash

set -u

folder=$(dirname "$0")

# Without the braces, the &> doesn't silence the floating-point exception
{ errors=$($TRILANGLE -w "${folder}/worse.trg" 2>&1 1<&-); } &>/dev/null
# For some reason, macOS 14 doesn't trigger a floating-point exception, so we can't verify a nonzero
# exit code.

set -e
test -n "$errors"
test -n "$($TRILANGLE -w "${folder}/bad.trg" 2>&1)"
