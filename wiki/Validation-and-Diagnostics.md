# Validation and Diagnostics

Deserialization can fail for many distinct reasons. The 1.0.0 API preserves detailed failure information rather than collapsing every problem into a generic parse failure.

Detailed diagnostics can distinguish:

- missing required properties;
- invalid value types;
- numeric constraint failures;
- unknown enum mappings;
- schema/migration failures;
- application-defined validation failures.

Use detailed deserialization facilities at boundaries where an operator, API caller, migration path, or test needs to understand why a representation was rejected.

## Validation order

Representation parsing establishes the archive structure, schema evolution brings older data toward the current schema, property conversion/defaults/aliases are applied according to contract, and model/application validation then decides whether the restored state is acceptable.

## Protected data

Protected serialization keeps security/authentication failures separate from archive/schema failures. Do not reinterpret an authentication failure as malformed plaintext data.