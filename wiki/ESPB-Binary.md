# ESPB Binary

ESPB Binary is ESPressio Serializable's native structured binary representation.

Use the normal Binary archive when data may outlive the exact firmware schema that produced it. The archive/tree path retains the structural information needed for aliases, defaults, validation, and schema migration.

## Persistence flow

```text
object
  -> declarative schema
  -> BinaryArchive / ESPB
  -> bytes
```

On restore:

```text
bytes
  -> BinaryArchive / ESPB
  -> migration / aliases / defaults / validation
  -> object
```

## Protected persistence

Optional authenticated protection wraps the complete finished ESPB representation rather than changing the ESPB format itself. See [Protected Serialization](Protected-Serialization).

## Bounded decoding

Always apply appropriate `BinaryArchiveDecodeLimits` for persisted or externally supplied bytes. Authentication proves integrity/origin under the configured key; it does not make an arbitrarily large authenticated object safe to allocate.