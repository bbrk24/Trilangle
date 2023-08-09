#!/bin/bash

set -e

folder="$(dirname "$0")"

int="$(trilangle "${folder}/int.trg")"
char="$(trilangle "${folder}/char.trg")"

test 1 = "$int"
test 1 = "$char"
