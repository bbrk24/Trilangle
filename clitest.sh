#!/bin/bash

set -ex

g++ -Og -std=gnu++17 src/*.cpp -o trilangle

for script in tests/cli/*/index.sh
do
    TRILANGLE=./trilangle "$script"
done

for script in tests/compiler/*/index.sh
do
    TRILANGLE=./trgc.sh "$script"
done
