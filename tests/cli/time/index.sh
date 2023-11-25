#!/bin/bash

set -eu

folder=$(dirname "$0")

time=$($TRILANGLE "${folder}/time.trg")

# This could be better.
test 0 -le "$time"
test $((0x7fffff)) -ge "$time"

# To avoid any possible confusion by the reported dates differing, wait if we're close to UTC midnight
if [ 23:59 = "$(date -u +%H:%M)" ] && [ 59 -le "$(date -u +%S)" ]
then
    sleep 2
fi

test $(($(date -u +%s) / (24 * 60 * 60))) = "$($TRILANGLE "${folder}/date.trg")"
