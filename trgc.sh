#!/bin/bash

set -euo pipefail
./trilangle -c "$@" | tee out.c | "${CC:-gcc}" -O1 -o ./trg.out -xc -
./trg.out
