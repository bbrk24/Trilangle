#!/bin/bash

set -e

folder="$(dirname "$0")"

warns1="$(trilangle -aw "${folder}/read.trg" 2>&1 <<<'é')"
# Technically it is possible to avoid writing this twice. But it's so convoluted that it's easier to not.
warns2="$(trilangle -aw "${folder}/write.trg" 2>&1 1<&-)"
out2="$(trilangle -aw "${folder}/write.trg" 2>/dev/null)"

nowarn="$(trilangle -aw "${folder}/read.trg" 2>&1 <<<'e')"
e=$(trilangle -aw "${folder}/fine.trg")

test -n "$warns1"
test -n "$warns2"
test 1 -eq "$(wc -m <<<"$out2")"
test -z "$nowarn"
test e = "$e"
