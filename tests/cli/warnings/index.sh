#!/bin/bash

set -uxo pipefail

folder=$(dirname "$0")

# Without the braces, the &> doesn't silence the floating-point exception
{ errors=$($TRILANGLE -w "${folder}/worse.trg" 2>&1 1<&-); } &>/dev/null
result=$?
set -e
test 0 -ne $result

test -n "$errors"
test -n "$($TRILANGLE -w "${folder}/bad.trg" 2>&1)"
