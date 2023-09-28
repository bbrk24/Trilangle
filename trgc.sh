#!/bin/bash

set -euo pipefail
./trilangle -c "$@" | "${CC:-gcc}" -o ./trg.out -xc -
./trg.out
