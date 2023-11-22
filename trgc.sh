#!/bin/bash

set -euo pipefail
./trilangle -c "$@" | tee out.c | "${CC:-gcc}" -o ./trg.out -xc -
./trg.out
