#!/bin/bash

set -eu

folder=$(dirname "$0")

test 1 = "$($TRILANGLE "${folder}/int.trg")"
test 1 = "$($TRILANGLE -a "${folder}/char.trg")"
