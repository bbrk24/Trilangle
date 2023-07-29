#!/bin/bash

set -e

folder="$(dirname "$0")"

errors="$(trilangle -w "${folder}/bad.trg" 2>&1)"

test -n "$errors"
