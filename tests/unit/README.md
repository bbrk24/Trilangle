# Unit Tests

This folder is for functions that can be tested easily and should not have side effects.

## Running the tests

Running the tests is simple:

```bash
shopt -s globstar extglob
g++ -Og -Isrc -std=c++17 tests/unit/**/*.cpp src/!(main).cpp
./a.out
```

### MSVC

Avoid compiling with the MSVC CLI if possible, as it's more involved than the alternatives. However, in Visual Studio developer PowerShell, this should work:

```pwsh
cl tests\unit\main.cpp tests\unit\test-framework\*\*.cpp (ls src\*.cpp -Exclude *main.cpp | % FullName) /Od /EHs /Isrc /std:c++17 /MTd /link /out:test.exe
.\test.exe
```
