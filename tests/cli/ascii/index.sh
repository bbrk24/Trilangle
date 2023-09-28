#!/bin/bash

set -eu

folder=$(dirname "$0")

test -n "$($TRILANGLE -aw "${folder}/read.trg" 2>&1 <<<'é')"
test -n "$($TRILANGLE -aw "${folder}/write.trg" 2>&1 1<&-)"

# This value will vary depending on the platform. Some will normalize it to a multi-byte substitution character.
# Similarly, using `-m` will cause an error on some platforms and report '1' on others.
bytecount=$($TRILANGLE -aw "${folder}/write.trg" 2>/dev/null | wc -c)
test 4 -ge "$bytecount"
test 1 -le "$bytecount"

test -z "$($TRILANGLE -aw "${folder}/read.trg" 2>&1 <<<'e')"
test e = "$($TRILANGLE -aw "${folder}/fine.trg")"
