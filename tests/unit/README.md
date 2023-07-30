# Unit Tests

This folder is for functions th+at can be tested easily and should not have side effects.

## Running the tests

Running the tests is simple:

```bash
shopt -s globstar extglob
g++ -Og -Isrc -- tests/unit/**/*.cpp src/!(main).cpp
./a.out
```
