# Bounded serialization validation

Source baseline: `ddd7ace2dc8b18953a9b0e36a47b12978bce40c1`, revalidated on `primitives_redesign` before changes. Foundation F04–F06 implement P3 from handoff Revision 079. No manifest version changes or new core dependencies.

## Existing test classification (reviewed before execution)

| Test | Classification and reason |
| --- | --- |
| test_compatibility | Retain: existing ESPB/CBOR scalar/name/version vectors remain valid. |
| test_diagnostics | Retain: general archive nested issue paths and fail-fast behavior. |
| test_direct_binary | Retain: general direct/tree agreement, optional fields, enums, nested values and prefixed output. |
| test_headers | Retain and extend via umbrella: public general headers remain usable. |
| test_hidden_copy | Retain: general node/archive ownership and move behavior. |
| test_malformed | Retain: general archive depth/count/length/node limits and deterministic malformed traversal stress. |
| test_migration_regression | Retain: historical migrations remain supported outside Primitive wire paths. |
| test_pool_allocator | Update: metadata heap allocation and shared-reference identity are superseded by static tuple derivation. General archive System allocation remains valid. |
| test_schema_formats | Retain: general human-facing introspection renderers. |
| test_security_integration | Retain: optional Security protection, authentication context and byte limits. |
| test_serializable | Retain: general container/default/range/alias/enum/read-only/redaction/migration behavior. |
| test_value_composition | Retain: serializable values remain ordinarily constructible/destructible/composable. |
| platformio/test_streaming | Retain: general ArduinoJson stream malformed/schema-filtered behavior; requires embedded toolchain. |
| fuzz/fuzz_binary, fuzz/fuzz_cbor | Retain: optional libFuzzer harnesses for general archive loaders. |

## Added coverage

`test_bounded` denies global heap allocation while exercising fixed containers, static schema descriptors, order-independent fingerprints, canonical ESPB/CBOR/JSON encoding and decoding, every truncation of valid messages, explicit current versions and nested capacity rejection. ESPB and CBOR encodings are compared byte-for-byte against the existing general archives.

`test_bounded_edge_cases` covers metadata alias collisions/overflow, nested optional distinctions, canonical read-only hydration, Unicode/surrogate/escaped-name behavior, embedded zero bytes, extreme floating point, integer overflow, duplicate/unknown members, and 2,000 deterministic generated values checked against every complete format maximum.

## Memory and work bounds

For each bounded vector, storage is `sizeof(std::array<T,N>) + sizeof(size_t)` plus alignment; all N elements are initialized. Bounded bytes use uint8_t elements. Bounded text uses N+1 bytes plus one count. Bounded maps/sets use fixed sequences and at most N comparisons per lookup/insertion.

One constexpr static property tuple, one constexpr static property descriptor array, and the reachable immutable value/schema descriptors are retained per instantiated schema/type. They never acquire an allocator or own an application object. Alias access is a fixed metadata thunk.

Encoding writes directly into a caller-owned buffer. Scalar JSON conversion uses a fixed 64-byte local buffer; no tree is built. Decoding uses one temporary T, fixed per-object seen bits, and one typed temporary per nesting level. Publication requires nonthrowing move assignment. JSON counting/validation uses recursion bounded by the schema-derived depth and scans only the already bounded input. Names and enum spellings are compared as views; escaped names are compared incrementally. Decoded JSON text length is checked before copying. No collection grows beyond its explicit capacity, and oversized total input is rejected before constructing the temporary object.

The declared bounds are conservative; unused space never enables a larger logical string/container. Defaults, application constructors/validators and other opaque business code must independently honor their owning Primitive contract. A tree-only SerializationAdapter does not prove bounded wire eligibility.

## Validation status

GCC 13.3 / C++17 CMake build and all 15 CTest gates passed: 14 native suites and one three-case expected-failure compile check. Nine new/changed headers compiled independently with warnings as errors and RTTI disabled. Both bounded README examples compiled and ran. AddressSanitizer and UndefinedBehaviorSanitizer passed the edge-case suite with leak detection disabled: LeakSanitizer cannot operate under this runtime’s ptrace environment and is classified unavailable locally. Heap-denial tests passed separately. GitHub host and ESP32 compile automation is pending publication of this commit; no hardware execution or quota failure is claimed.
