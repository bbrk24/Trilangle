#!/bin/bash

set -e

folder="$(dirname "$0")"

with_nops="$(trilangle -D "${folder}/nopful.trg")"
no_nops="$(trilangle -Dn "${folder}/nopful.trg")"

test 6 -eq "$(wc -l <<<"$with_nops")"
test 1 -eq "$(wc -l <<<"$no_nops")"
