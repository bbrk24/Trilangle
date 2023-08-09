#!/bin/bash

set -e

folder="$(dirname "$0")"

sixteen="$(trilangle "${folder}/sixteen.trg")"
big_negative="$(trilangle "${folder}/big_negative.trg")"

test 16 = "$sixteen"
test "$big_negative" -eq $((-0x800000))
