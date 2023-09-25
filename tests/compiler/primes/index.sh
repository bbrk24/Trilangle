#!/bin/bash

set -euo pipefail

folder=$(dirname "$0")

trilangle -c "${folder}/primes.trg" | $CC -o ./trgcat.out

one=$(./trgcat.out <<<1)
two=$(./trgcat.out <<<2)
four=$(./trgcat.out <<<4)

test -z "$one"
test 0 = "$two"
test -z "$four"
