# Nested Values and Collections

Serializable supports nested model composition and common C++ collection/value forms.

Typical supported shapes include nested Serializable objects, arithmetic values, strings/Arduino `String`, arrays and standard containers, `std::optional`, enums, maps, and sets where the selected archive supports them.

## Composition

Prefer normal C++ value composition: a Serializable object can contain another Serializable value as a member and collections can contain supported value types.

## Resource bounds

Nested/collection support can amplify hostile or corrupted input. Archive decode limits should constrain nesting depth, total nodes, object members, array elements, property-name length, and string length.

## Schema design

Use nested objects when they represent a meaningful sub-model. Do not flatten structure merely to simplify one representation, because the declarative schema should remain representation-neutral.