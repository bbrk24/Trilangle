#!/bin/bash

set -e

folder="$(dirname "$0")"

int="$(trilangle "${folder}/int.trg")"
char="$(trilangle "${folder}/char.trg")"

test "$int" = 1
test "$char" = 1
