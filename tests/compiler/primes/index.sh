#!/bin/bash

set -eu

folder=$(dirname "$0")

test -z "$($TRILANGLE "${folder}/primes.trg" <<<1)"
test 0 = "$($TRILANGLE "${folder}/primes.trg" <<<2)"
test -z "$($TRILANGLE "${folder}/primes.trg" <<<4)"
