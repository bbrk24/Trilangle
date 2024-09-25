#!/bin/bash

set -ex

# Some of the examples (particularly the Qdeql truth machine) run very slowly
# if I/O buffering is enabled.
g++ -Og -DNO_BUFFER -std=gnu++23 src/*.cpp -o trilangle

for script in tests/cli/*/index.sh
do
    TRILANGLE=./trilangle "$script"
done

for script in tests/compiler/*/index.sh
do
    TRILANGLE=./trgc.sh "$script"
done
