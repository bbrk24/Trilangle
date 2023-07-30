# Unit Tests

This folder is for functions that can be tested easily and should not have side effects.

## Running the tests

Running the tests is simple:

```sh
shopt -s globstar # bash-only; not necessary in zsh
g++ -Og tests/unit/**/*.cpp -Isrc src/output.cpp src/program.cpp # or whichever compiler you prefer
./a.out
```

<!-- Note for future reference: If .cpp files from src/ are required in the future, don't naively use src/*.cpp, as that will get src/main.cpp and cause conflicts! -->
