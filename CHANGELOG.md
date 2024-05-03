# Changelog

All notable changes to this project will be documented in this file. See [Keep a Changelog] for formatting guidelines.

## Unreleased

### Changed

- Modified a test so it passes on macOS 14.
- Removed some more `constexpr` annotations I missed last time.

## [1.8.1] - 2024-05-03

### Changed

- Removed some `constexpr` annotations so the code compiles on macOS.

## [1.8.0] - 2024-05-03

### Added

- The language version is now displayed on the online interpreter.
- RND (`$`) now works in compiled programs, though backed by a different source of randomness.

### Changed

- Fixed the position of the variation selector on the pause button.
- Changed the way new colors are generated for the web interface.

## [1.7.0] - 2024-01-23

### Changed

- The online interpreter no longer interprets shebangs. For compatibility with the CLI, the "condense" button still leaves whitespace in programs that start with `#!`.
- Some error messages have been reworded.
- Overflow checking on most arithmetic operations is now laxer to allow for unsigned numbers.

### Fixed

- Fixed a long-standing rounding bug in `GTM` that affected times shortly before UTC midnight.
- Added some missing newlines to the help string.
- Modifying the input when "include input in URL" is checked now invalidates the generated URL.

### Added

- Added UDV and PTU opcodes (`d` and `p`).
- GDT and GTM (`D` and `T`) now work in compiled programs.

## [1.6.2] - 2023-11-21

### Fixed

- The EXP opcode now behaves consistently between the interpreter and the C compiler.

### Added

- You can now customize the colors of the threads in the online debugger.

## [1.6.1] - 2023-10-24

### Fixed

Corrected overloads so it now compiles on macOS.

## [1.6.0] - 2023-10-24

### Added

- Added prebuilt binaries for ARM64 macOS, x86 Windows, and ARM64 Windows.
- Added a 'play' feature to the online debugger, to automatically step through the code.
- Added a compile mode to emit C code.
- Added `-a` flag to assume ASCII I/O.

## [1.5.0] - 2023-08-31

### Added

- Added a low-data version of the online interpreter.
- Added the `-z` flag to read the program as null-terminated. This is useful for passing both the program and its input over stdin.
- It is now possible to include the contents of stdin in the URL.

### Fixed

- The debug highlight now respects non-breaking spaces in the source.
- The debug pop-up now line wraps correctly on smaller screens.

## [1.4.2] - 2023-07-29

### Fixed

- The online interpreter correctly resets STDIN each time.
- The IP highlight now always renders in the correct place while debugging.

## [1.4.1] - 2023-07-26

### Fixed

- The "Step" button in the GUI debugger now works correctly on iOS.
- The IP highlight now renders in the correct position on Chrome and related browsers.

## [1.4.0] - 2023-07-26

### Added

- Added a `--version` flag. The version must be defined when compiling the interpreter, with `-DVERSION=...` (gcc/clang) or `/DVERSION=...` (MSVC).
- Added a GUI debugger to the web interface.

## [1.3.5] - 2023-06-11

No code changes, just fixing CI.

## [1.3.4] - 2023-06-11

### Fixed

- Changed the overloads of `int24_t::int24_t` so that it compiles on macOS.

## [1.3.3] - 2023-06-11

### Fixed

- Added a missing `#include` required to build on Linux.

## [1.3.2] - 2023-06-11

No code changes, just fixing CI.

## [1.3.1] - 2023-06-11

### Changed

- Overflow checking for multiplication on x86-64 now uses the `seto` instruction rather than preempting it with division.

### Fixed

- Corrected the warning condition for overflow with DEC.

## [1.3.0] - 2023-05-21

### Added

- Added 2-dupe instruction (DP2, `z`).

### Fixed

- Fixed bug in disassembler where it would emit extraneous numbers in some cases.
- Fit help string within 80 characters per line.
- Improved contrast for link and error colors in the web interface.
- Fixed a bug involving background colors in the web interface in Safari.

### Changed

- Changed disassembler label format from a single integer to two integers separated by a dot.
- Uses EXIT_FAILURE instead of hardcoding 1.

## [1.2.1] - 2023-03-28

### Added

- The web interface now has toggles for light/dark and high-contrast/low-contrast color schemes.

### Fixed

- The input box now fits on the screen on small mobile devices.
- Invalid URLs no longer prevent the page from laying out correctly.

## [1.2.0] - 2023-03-21

### Changed

- Gave the web interface a complete redesign.

### Added

- Added GDT and GTM opcodes.
- Added custom URL generation for the online interpreter.

### Fixed

- Greatly improved execution speed of code that produces a lot of output.
- Long-running loops that produce no output can now be stopped.
- Fixed shebang handling when condensing code.
- Fixed handling of trailing `.`s when condensing code.

## [1.1.1] - 2023-03-15

### Fixed

- Minor performance improvements with I/O, especially for the web interface.
- Fixed a race condition in loading the web interface.

### Added

- Added a warning for invalid characters.

### Changed

- The disassembler no longer continues after an invalid opcode.
- The program and input fields in the web interface are larger.

## [1.1.0] - 2023-03-12

### Fixed

- Some UTF-8 handling edge cases have been fixed.

### Added

- Added a `--expand` flag to add spaces to code, and a corresponding button to the web interface.
- There are now threading operators, `{` and `}`.

### Changed

- Various improvements have been made to the CSS for the web interface.

## [1.0.0] - 2023-03-02

First versioned release.

[Keep a Changelog]: https://keepachangelog.com/en/
[1.0.0]: https://github.com/bbrk24/Trilangle/tree/1.0.0
[1.1.0]: https://github.com/bbrk24/Trilangle/tree/1.1.0
[1.1.1]: https://github.com/bbrk24/Trilangle/tree/1.1.1
[1.2.0]: https://github.com/bbrk24/Trilangle/tree/1.2.0
[1.2.1]: https://github.com/bbrk24/Trilangle/tree/1.2.1
[1.3.0]: https://github.com/bbrk24/Trilangle/tree/1.3.0
[1.3.1]: https://github.com/bbrk24/Trilangle/tree/1.3.1
[1.3.2]: https://github.com/bbrk24/Trilangle/tree/1.3.2
[1.3.3]: https://github.com/bbrk24/Trilangle/tree/1.3.3
[1.3.4]: https://github.com/bbrk24/Trilangle/tree/1.3.4
[1.3.5]: https://github.com/bbrk24/Trilangle/tree/1.3.5
[1.4.0]: https://github.com/bbrk24/Trilangle/tree/1.4.0
[1.4.1]: https://github.com/bbrk24/Trilangle/tree/1.4.1
[1.4.2]: https://github.com/bbrk24/Trilangle/tree/1.4.2
[1.5.0]: https://github.com/bbrk24/Trilangle/tree/1.5.0
[1.6.0]: https://github.com/bbrk24/Trilangle/tree/1.6.0
[1.6.1]: https://github.com/bbrk24/Trilangle/tree/1.6.1
[1.6.2]: https://github.com/bbrk24/Trilangle/tree/1.6.2
[1.7.0]: https://github.com/bbrk24/Trilangle/tree/1.7.0
[1.8.0]: https://github.com/bbrk24/Trilangle/tree/1.8.0
[1.8.1]: https://github.com/bbrk24/Trilangle/tree/1.8.1
