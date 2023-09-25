#!/bin/bash

set -euo pipefail

folder=$(dirname "$0")

trilangle -c "${folder}/primes.trg" | $CC -o ./trgprimes.out -xc -

one=$(./trgprimes.out <<<1)
two=$(./trgprimes.out <<<2)
four=$(./trgprimes.out <<<4)

test -z "$one"
test 0 = "$two"
test -z "$four"
