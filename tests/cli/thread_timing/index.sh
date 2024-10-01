#!/bin/bash

set -eu

folder=$(dirname "$0")

for file in "$folder"/*.trg
do
    output=$($TRILANGLE -a "$file")
    test "${output//$'\r'/}" = $'1\n2'
done
