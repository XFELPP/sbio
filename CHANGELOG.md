# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.1] - 2026-08-05

### Fixed
- Fixed incorrect maximum rank in data spec
- Address potential nullptr dereference in SubDivide partitioning strategy when missing segments.
- Fixed name collision of SyncPOSIXIO in Python bindings causing import errors even though different actual classes.

### Changed
- Autosort and begin indexing from 0 for the segments in SubDivide partitioning strategy in the BrokerGroup for nullptr guard. Potentially may cause non-optimal read patterns since the fetch ordering changes but will have to see.

## [0.1.0] - 2026/08/01
- Initial release
