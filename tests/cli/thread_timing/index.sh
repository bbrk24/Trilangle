#!/bin/bash

set -euo pipefail

folder=$(dirname "$0")

for file in "$folder"/*.trg
do
    output=$($TRILANGLE -a "$file")
    test "${output//$'\r'/}" = $'1\n2'

    if ! grep -q '#' "$file"
    then
        output=$($TRILANGLE -D "$file" | $TRILANGLE -Aa)
        test "${output//$'\r'/}" = $'1\n2'
    fi
done
