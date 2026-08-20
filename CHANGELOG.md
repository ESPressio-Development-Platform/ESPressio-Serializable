# Changelog

All notable changes to this project are documented in this file.

The structure follows the principles of [Keep a
Changelog](https://keepachangelog.com/en/1.1.0/) and [Semantic
Versioning](https://semver.org/).

> **Historical note:** This changelog was reconstructed retrospectively
> from published GitHub Releases, tags, release notes, repository
> history, and the documented public API. Where an historical release
> had little or no release-note detail, the entry is intentionally terse
> rather than inferring unsupported intent.

## [0.10.0] - 2026-08-20

### Added

-   Added `ESPressio_DirectBinaryArchive.hpp`.
-   Added `SerializeDirectBinary()` for direct schema/property-to-byte
    serialization without a full intermediate `SerializationNode` tree.
-   Added `DeserializeDirectBinary()` for direct same-schema binary decoding.
-   Added `AppendDirectBinary()` so protocol integrations can serialize into an
    already-prefixed/final output buffer.
-   Added compatibility tests covering nested Serializable values, containers,
    optional values, enum mappings, maps, malformed input, and byte-for-byte
    equivalence with BinaryArchive ESPB v2 output.

### Changed

-   Exported the direct Binary API through `ESPressio_Serializable.hpp`.
-   Preserved the existing BinaryArchive ESPB v2 wire representation so the new
    fast path does not require a protocol-version change.
-   Kept the existing BinaryArchive/TreeArchive path available for structural
    schema migration and compatibility fallback.

## [0.9.0] - 2026-08-18

### Added

-   First public pre-release intended for downstream integration and
    real-world validation.
-   Completed the planned 1.0 feature scope for declarative
    serialization and schema handling.
-   Added/solidified JSON, CBOR, and direct Binary archive support.
-   Added nested Serializable objects and container support.
-   Added Arduino `String` support without making the portable core
    Arduino-dependent.
-   Added `std::optional`, map/set support, and enum mapping facilities.
-   Added redaction metadata.
-   Added numeric and application-defined validation.
-   Added schema versions, aliases, default values, and migration
    helpers.
-   Added streaming parser/writer facilities.
-   Added richer compile-time diagnostics.
-   Added allocator strategy support.
-   Added schema/documentation facilities.
-   Added compatibility vectors, fuzz-test infrastructure, and
    benchmarking support.

### Changed

-   Declared the public API provisionally frozen for the 0.9.x
    validation cycle.
-   Shifted development focus toward hardware validation, compatibility
    testing, fuzzing, benchmarking, documentation, API consistency, and
    defect correction ahead of 1.0.0.

## Pre-0.9 development milestones

The repository evolved rapidly through internal/pre-release 0.x
milestones before the first published GitHub Release at 0.9.0.

### Added

-   Established the CRTP/property-descriptor Serializable foundation.
-   Added declarative property schemas and schema-version metadata.
-   Added JSON serialization/deserialization.
-   Added CBOR and Binary representations.
-   Added nested objects and array/container handling.
-   Added validation, migrations, aliases, defaults, redaction,
    streaming, and extensibility in successive development iterations.

> The current GitHub Releases history publishes 0.9.0 as the first
> release. Earlier 0.x version numbers appeared during repository
> development, but are grouped here rather than assigning release dates
> or exact contents that are not fully supported by the published
> release record.
