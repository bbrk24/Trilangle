# Changelog

All notable changes to this project will be documented in this file. See [Keep a Changelog] for formatting guidelines.

## Unreleased

### Fixed

- The input box now fits on the screen on small mobile devies.
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
