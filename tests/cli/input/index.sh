#!/bin/bash

set -eu

folder=$(dirname "$0")

test 1 = "$($TRILANGLE "${folder}/int.trg" <<<1)"
test 2 = "$($TRILANGLE "${folder}/int.trg" <<<'some junk and then a 2')"
test 3 = "$($TRILANGLE "${folder}/int.trg" <<<'3 with some trailing junk')"
test 16 = "$($TRILANGLE "${folder}/int.trg" <<<'0x10')"
test -1 = "$($TRILANGLE "${folder}/int.trg" <<<'')"
