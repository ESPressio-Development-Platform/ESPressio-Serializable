# Testing Serializable Extensions

Serializable tests should cover model/schema behaviour independently from each archive, then verify representation-specific behaviour and integrations.

## Core schema

Test property discovery, required/default/alias semantics, nested values, collections, optional values, enum mappings, validation, copy/move/value composition, and schema versions.

## Archives

For each representation, test round trips, malformed input, unsupported shapes, structural limits, numeric boundaries, and detailed error reporting.

## Evolution

Maintain compatibility fixtures for prior supported schema versions and verify migrations/defaults/aliases into current models.

## Streaming

Exercise every possible partial read/write boundary, early EOF, transport failure, and restart/cleanup path.

## Security

Test authenticated protected round trips, wrong context/key, tampering, protection errors versus deserialization errors, and bounded decoded archive size.

## Memory

Instrument allocation where possible to verify configured serialization allocators are honoured, prefer-PSRAM fallback works, and decode limits prevent unbounded growth.