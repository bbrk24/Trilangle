#!/bin/bash
echo 'Beginning script...'

set -e

text='
    /\_/\           ___
   = ಠ_ಠ =_______    \ \
    __^      __(  \.__) )
(@)<_____>__(_____)____/'

folder="$(dirname "$0")"
echo "Script location: $folder. Running script..."
output="$(trilangle "${folder}/cat.trg" <<<"$text")"
echo "Script ran successfully. Output: $output"

test "$text" = "$output"
