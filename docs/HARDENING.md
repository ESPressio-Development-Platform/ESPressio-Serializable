# Hardening and malformed-input policy

The Binary and CBOR decoders are exercised by deterministic mutation tests on every host test run and have libFuzzer entry points under `tests/fuzz/`. Recommended CI uses both AddressSanitizer and UndefinedBehaviorSanitizer in addition to the normal CTest suite.

Malformed input must fail without reading beyond the supplied buffer, performing unbounded allocation, or partially treating invalid framing as a valid object. Native Binary readers reject unknown wire-version bytes. Streaming readers should expose malformed input through `DeserializationResult` rather than silently accepting incomplete data.

`DeserializationOptions` controls validation behavior. `CollectAll` aggregates nested property/index paths up to `MaximumIssues`; `FailFast` stops at the first reported issue. This is intended both for developer diagnostics and for constrained devices where collecting a long issue list would be wasteful.

Fuzz corpora are append-only diagnostic assets: when a crash or parser defect is found, retain the triggering input (or a minimized equivalent) as a regression seed.
