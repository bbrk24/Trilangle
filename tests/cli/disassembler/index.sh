#!/bin/bash

set -uo pipefail

folder=$(dirname "$0")

# https://stackoverflow.com/a/20460402/6253337
string_contain () {
    case "$2" in
        *"$1"*)
            return 0
            ;;
        *)
            return 1
            ;;
    esac
}

tsp_disassembly=$($TRILANGLE -D "${folder}/tsp.trg")

if string_contain BNG "$tsp_disassembly"
then
  exit 1
fi

set -e

string_contain TSP "$tsp_disassembly"

test 6 -eq "$($TRILANGLE -D "${folder}/nopful.trg" | wc -l)"
test 1 -eq "$($TRILANGLE -Dn "${folder}/nopful.trg" | wc -l)"

cat_disassembly=$($TRILANGLE -Dn "${folder}/cat.trg")
test "${cat_disassembly//$'\r'/}" = $'0.1:\tGTC
0.2:\tBNG 2.0
1.0:\tPTC
1.1:\tPOP
1.5:\tJMP 0.1
2.0:\tPOP
2.2:\tEXT'
