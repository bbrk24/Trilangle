#!/bin/bash

set -e

folder="$(dirname "$0")"

sixteen="$(trilangle "${folder}/sixteen.trg")"
minus_one="$(trilangle "${folder}/minus_one.trg")"

test "$sixteen" = 16
test "$minus_one" = -1
