# Direct Binary

Direct Binary is the latency-sensitive same-schema fast path.

```cpp
std::vector<uint8_t> bytes;
Serializable::SerializeDirectBinary(object, bytes);
Serializable::DeserializeDirectBinary(
    bytes.data(),
    bytes.size(),
    object
);
```

## When to use it

Use direct Binary when both endpoints share the exact expected schema and minimizing archive/tree overhead matters.

## When not to use it

Do not make direct Binary the default persistence format for data expected to survive firmware schema evolution. The normal ESPB archive path provides the structural migration/default/alias facilities required for that use case.

## Compatibility

The direct path preserves the ESPB v2 byte compatibility defined by the 1.0.0 implementation, but its semantic contract remains same-schema fast-path use rather than general migration-oriented persistence.