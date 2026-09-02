# JSON Archives

JSON archives provide human-readable/interoperable serialization from the same declarative model used by every other representation.

Use JSON when readability, browser/tool interoperability, or external API compatibility matters more than compactness.

## Schema remains authoritative

Property names, value types, defaults, aliases, validation, and migrations come from the Serializable schema rather than being duplicated in JSON-specific model code.

## Diagnostics

Detailed deserialization should distinguish malformed JSON/representation errors from schema-level failures such as missing required properties, invalid types, numeric constraint failures, enum mapping errors, and application validation failures.

## Embedded considerations

JSON generally has higher byte and parsing overhead than CBOR/ESPB. Keep decode limits and input bounds appropriate to the target and trust boundary.