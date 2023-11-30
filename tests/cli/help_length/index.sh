#!/bin/bash

set -euo pipefail

test 80 -gt "$($TRILANGLE --help | expand -t4 | awk '{print length}' | sort -nr | head -n1)"
