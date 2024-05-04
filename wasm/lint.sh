#!/bin/bash

shopt -s globstar
set -e

engine-check
stylelint ./*.scss
eslint ./*.civet

cd ..
clang-format --Werror --dry-run src/*.cpp src/**/*.hh tests/unit/*.cpp tests/unit/*.hh
