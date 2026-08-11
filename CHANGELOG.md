# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.4] - 2026-08-10

### Added
- Create per-platform bundles of shared libraries independently of the Python wheels.
- Add `pkg-config` file generation. Python wheels provide a small command-line utility to find it.

### Changed
- Move to version `v0.7.5` of `ncarray`
- Remove `soversion` specifier from Windows builds due to difficulty with naming of import libraries (`.lib`s).
- Run the CI builds with a true split stage building C++ libraries first followed by solely Python with cibuildwheel.

### Fixed
- Fixed broken Python 3.8 wheel on macOS where extension module was partially bundled as Python 3.14.

## [0.1.3] - 2026-08-07

### Fixed
- Correct lib directory search strategy to find macOS delocate wheel-repaired libraries

### Changed
- Removed bleed-through `open-mpi` in CI wheels for macOS.

## [0.1.2] - 2026-08-06

### Fixed
- Import `ncarray` in the top of the Python module (not the C++ bindings) to make sure the ncarray libraries can be found by linker.

## [0.1.1] - 2026-08-05

### Fixed
- Fixed incorrect maximum rank in data spec
- Address potential nullptr dereference in SubDivide partitioning strategy when missing segments.
- Fixed name collision of SyncPOSIXIO in Python bindings causing import errors even though different actual classes.

### Changed
- Autosort and begin indexing from 0 for the segments in SubDivide partitioning strategy in the BrokerGroup for nullptr guard. Potentially may cause non-optimal read patterns since the fetch ordering changes but will have to see.

## [0.1.0] - 2026/08/01
- Initial release
