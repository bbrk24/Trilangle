#!/bin/bash

set -u

errors="$($TRILANGLE - 2>&1 1<&-)"
result=$?
set -e
test 0 -ne $result

test -n "$errors"
