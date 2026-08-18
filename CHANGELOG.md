# Changelog

## 0.9.0

Pre-release API-freeze milestone for downstream integration and hardware validation before `1.0.0`.

### Status

* Feature-complete for the planned `1.0.0` scope.
* Public API provisionally frozen.
* Focus shifted from feature development to compatibility, fuzzing, hardware validation, benchmarks, documentation, and downstream testing.

### Included capability areas

* declarative property metadata, defaults, validation, aliases, ranges, read-only/sensitive flags;
* schema versions, migration helpers, and migration regression fixtures;
* nested objects and standard collection support;
* enum string mappings;
* JSON, CBOR, native Binary, NVS, and Stream adapters;
* direct/streaming serialization and large-payload deserialization paths;
* structured diagnostics and configurable validation behavior;
* configurable allocators and PSRAM/static-pool strategies;
* schema introspection and documentation exporters;
* constrained binary/property-name-elision mode;
* malformed-input hardening, fuzz harnesses, and frozen compatibility vectors;
* ESP-NOW, Serial, TCP, CRC32-framed transport examples/adapters;
* reproducible ESP32 benchmark harnesses.

### Pre-1.0 validation goals

* physical ESP32/ESP32-S3/ESP32-C3 testing;
* Arduino IDE verification;
* downstream clean-install verification;
* longer fuzz campaigns;
* real RAM/flash/runtime measurements;
* final API/wire-format review.


