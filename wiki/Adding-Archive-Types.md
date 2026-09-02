# Adding Archive Types

Add a new archive when a new representation is required while the model schema should remain unchanged.

## Responsibilities

An archive implementation owns representation-specific encoding/decoding, structural traversal, representation parse/format errors, and any representation-specific streaming behaviour.

It must reuse the common schema/property/value model for names, value types, defaults, aliases, migrations, validation and redaction semantics.

## Requirements

A new archive should:

- support the value shapes it claims explicitly;
- reject unsupported shapes rather than coercing them unpredictably;
- preserve detailed error separation;
- honour redaction where applicable;
- apply appropriate structural/resource limits;
- support deterministic round trips for the documented schema subset;
- avoid adding dependencies to the core umbrella unless the archive itself is explicitly selected.

## Streaming archives

If the representation is incremental, document partial I/O, end-of-stream and failure semantics independently from schema failures.

## Testing

Create cross-representation fixtures where useful so the same model semantics are validated through the new archive and existing archives.