#!/bin/bash
# Note: 'npx' should be omitted in this file

shopt -s globstar
set -e

engine-check
stylelint ./*.scss

civet -c ./*.civet -o .ts
eslint ./*.ts

cd ..
clang-format --Werror --dry-run src/*.cpp src/**/*.hh tests/unit/*.cpp tests/unit/*.hh
