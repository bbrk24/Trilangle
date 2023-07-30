#!/bin/bash

set -e

folder="$(dirname "$0")"

one="$(trilangle "${folder}/int.trg" <<<1)"
two="$(trilangle "${folder}/int.trg" <<<'some junk and then a 2')"
three="$(trilangle "${folder}/int.trg" <<<'3 with some trailing junk')"
sixteen="$(trilangle "${folder}/int.trg" <<<'0x10')"
minus_one="$(trilangle "${folder}/int.trg" <<<'')"

test "$one" = 1
test "$two" = 2
test "$three" = 3
test "$sixteen" = 16
test "$minus_one" = -1
