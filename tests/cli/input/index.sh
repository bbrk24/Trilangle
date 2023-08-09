#!/bin/bash

set -e

folder="$(dirname "$0")"

one="$(trilangle "${folder}/int.trg" <<<1)"
two="$(trilangle "${folder}/int.trg" <<<'some junk and then a 2')"
three="$(trilangle "${folder}/int.trg" <<<'3 with some trailing junk')"
sixteen="$(trilangle "${folder}/int.trg" <<<'0x10')"
minus_one="$(trilangle "${folder}/int.trg" <<<'')"

test 1 = "$one"
test 2 = "$two"
test 3 = "$three"
test 16 = "$sixteen"
test -1 = "$minus_one"
