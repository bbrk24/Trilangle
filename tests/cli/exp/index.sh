#!/bin/bash

set -eu

folder=$(dirname "$0")

test 16 = "$($TRILANGLE "${folder}/sixteen.trg")"
test "$($TRILANGLE "${folder}/big_negative.trg")" -eq $((-0x800000))
