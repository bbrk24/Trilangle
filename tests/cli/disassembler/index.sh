#!/bin/bash

set -euo pipefail

folder=$(dirname "$0")

test 6 -eq "$($TRILANGLE -D "${folder}/nopful.trg" | wc -l)"
test 1 -eq "$($TRILANGLE -Dn "${folder}/nopful.trg" | wc -l)"
