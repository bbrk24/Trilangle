#!/bin/bash

set -e

folder="$(dirname "$0")"

warns1="$(trilangle -aw "${folder}/read.trg" 2>&1 <<<'é')"

# Technically it is possible to avoid writing this twice. But it's so convoluted that it's easier to not.
warns2="$(trilangle -aw "${folder}/write.trg" 2>&1 1<&-)"
# This value will vary depending on the platform. Some will normalize it to a multi-byte substitution character.
# Similarly, using `-m` will cause an error on some platforms and report '1' on others.
bytecount="$(trilangle -aw "${folder}/write.trg" 2>/dev/null | wc -c)"

nowarn="$(trilangle -aw "${folder}/read.trg" 2>&1 <<<'e')"
e=$(trilangle -aw "${folder}/fine.trg")

test -n "$warns1"
test -n "$warns2"
test 4 -ge "$bytecount"
test 1 -le "$bytecount"
test -z "$nowarn"
test e = "$e"
