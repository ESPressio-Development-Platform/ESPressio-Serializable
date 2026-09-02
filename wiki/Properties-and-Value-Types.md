# Properties and Value Types

Serializable properties can represent primitive values and richer C++ value shapes supported by the selected archive.

The 1.0.0 baseline supports common arithmetic values, strings and Arduino `String`, nested Serializable objects, arrays/standard containers, `std::optional`, enums, maps, and sets where the archive supports the required structure.

## Representation neutrality

A property declaration should describe the C++ state, not pre-format it for JSON or another archive.

## Enums

Use the library's enum mapping facilities rather than relying on compiler-specific numeric representations when stable human/schema names are needed.

## Optional values

`std::optional` expresses absence explicitly. Do not invent sentinel numeric/string values solely for serialization.

## Collections

Collection support should preserve element value semantics and respect archive/resource limits. For untrusted data, bounded decoding remains essential regardless of the C++ container's ability to grow.