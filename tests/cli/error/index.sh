#!/bin/bash

set -u

folder=$(dirname "$0")

errors=$($TRILANGLE "${folder}/error.trg" 2>&1 1<&-)
result=$?
set -e
test 0 -ne $result
test -n "$errors"
