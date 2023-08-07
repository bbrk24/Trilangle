#!/bin/bash

folder="$(dirname "$0")"

errors="$(trilangle "${folder}/error.trg" 2>&1 1<&-)"
result=$?
set -e
test $result -ne 0
test -n "$errors"
