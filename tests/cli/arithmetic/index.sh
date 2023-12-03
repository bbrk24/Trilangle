#!/bin/bash

set -eu

folder=$(dirname "$0")

five=$($TRILANGLE "${folder}/add.trg" <<<'3 2')
one=$($TRILANGLE "${folder}/sub.trg" <<<'3 2')
minustwo=$($TRILANGLE "${folder}/sdiv.trg" <<<'-5 2')
twopowtwentytwo=$($TRILANGLE "${folder}/udiv.trg" <<<"$((0x800000)) 2")

test 5 = "$five"
test 1 = "$one"
test -2 = "$minustwo"
test $((0x400000)) = "$twopowtwentytwo"
