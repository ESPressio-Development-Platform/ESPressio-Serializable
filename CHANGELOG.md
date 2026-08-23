# Changelog

All notable changes to this project are documented in this file.

The structure follows the principles of Keep a Changelog and Semantic Versioning.

## [0.11.2] - 2026-08-23

### Fixed

- Corrected package version metadata so PlatformIO and Arduino package tooling identify the release as `0.11.2` rather than the stale `0.11.0` value inherited by the 0.11.1 tag.
- Aligned `library.json`, `library.properties`, README installation guidance and release documentation with the actual patch version.

### Compatibility

- No public API, serialization format, schema, runtime behaviour or dependency changes.
- Includes the public `SerializableBase<TDerived>` destructor correction introduced in 0.11.1.
- Existing 0.11.x consumers remain source- and wire-compatible.

### Tracking

- Fixes #21.

## [0.11.0] - 2026-08-23

### Added

- Added optional `ESPressio_Serializable_Security.hpp` integration with ESPressio Security 0.4.x.
- Added `SerializationProtectionConfig` to encapsulate a configured `IDataProtector`, authenticated purpose/context, archive size limit, BinaryArchive decode limits and deserialization options.
- Added `SerializeProtectedBinary()` and `DeserializeProtectedBinary()` for authenticated protection of complete ESPB BinaryArchive representations.
- Added configuration-driven `SerializeBinary()` / `DeserializeBinary()` helpers where a null protection pointer preserves the ordinary unprotected ESPB path.
- Added `ProtectedSerializationResult` and `ProtectedSerializationStatus`, preserving Security failures separately from Serializable deserialization diagnostics.
- Added host coverage for protected round trips, authenticated-context mismatch, unprotected compatibility and resource limits.
- Added a complete ESP32 protected-binary example.

### Design

- Protection is applied to the finished archive representation, not to individual properties. This preserves the authoritative Serializable schema and BinaryArchive migration path while giving the complete representation confidentiality and integrity/authenticity.
- Redaction remains independent: redaction protects diagnostics/log output; authenticated encryption protects represented bytes.
- Security remains optional. Core `ESPressio_Serializable.hpp` does not include or require ESPressio Security.

### Compatibility

- Backward-compatible public API extension from 0.10.3 to 0.11.0.
- Existing JSON, CBOR, ESPB Binary, direct Binary, streaming, schema and migration APIs remain unchanged.
- ESPB v2 wire format is unchanged before optional protection is applied.

### Tracking

- Implements #17.

## [0.10.3] - 2026-08-22

### Changed

- Published the post-migration package generation from the dedicated `ESPressio-Development-Platform` GitHub organization.
- Updated package identity and documentation to use `https://espressio.org` and the relocated repository coordinates directly rather than relying on legacy organization redirects.

### Compatibility

- No public API, wire-format, or runtime behaviour changes.
- ESPB v2 compatibility is unchanged.
- This patch establishes the migrated Serializable baseline for the downstream ESPressio dependency-release cascade.

## [0.10.2] - 2026-08-21

### Fixed

- Made fallback detailed-deserialization control flow explicit in `ESPressio_SerializationTraversal.hpp`, eliminating GCC's `-Wmisleading-indentation` diagnostic when downstream consumers compile ESPressio Serializable headers with warnings treated as errors.

### Compatibility

- No API, wire-format, or runtime behaviour changes.
- ESPB v2 compatibility is unchanged.

## [0.10.1] - 2026-08-20

### Fixed

- Hardened `BinaryArchive::Load()` against malformed and adversarial ESPB v2 payloads with bounded depth, total nodes, object members, array elements, property-name lengths and string lengths.
- Added overflow-safe buffer validation and exception-safe invalid-archive handling.

### Added

- Added `BinaryArchiveDecodeLimits`.
- Added `BinaryArchiveVisitor`, `TraverseBinaryArchive()` and `ValidateBinaryArchive()` for bounded allocation-free ESPB inspection.

## [0.10.0] - 2026-08-20

### Added

- Added direct ESPB Binary serialization/deserialization APIs and compatibility coverage.

### Changed

- Preserved the existing BinaryArchive ESPB v2 representation and migration-capable tree/archive path.

## [0.9.0] - 2026-08-18

### Added

- First public pre-release intended for downstream integration and real-world validation.
- Completed the planned declarative serialization/schema scope including JSON, CBOR, Binary, nested values, collections, Arduino `String`, optional/map/set/enum support, redaction, validation, schema evolution, streaming, allocator strategies, diagnostics and fuzz/benchmark infrastructure.

## Pre-0.9 development milestones

The repository evolved rapidly through internal/pre-release 0.x milestones before the first published GitHub Release at 0.9.0. Earlier development established the CRTP/property-descriptor foundation and the representation-neutral archive model.
