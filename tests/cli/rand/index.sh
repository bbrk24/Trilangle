#!/bin/bash

set -eu

folder=$(dirname "$0")

for _ in {0..10}
do
    rand=$($TRILANGLE "${folder}/rng.trg")

    test $((-0x800000)) -le "$rand"
    test $((0x7fffff)) -ge "$rand"
done
