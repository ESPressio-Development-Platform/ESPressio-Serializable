# Adding Value Adapters

Value adapters extend which C++ value shapes can participate in Serializable schemas without teaching every archive about application-specific model types.

## Responsibilities

A value adapter should define how a C++ value maps to the representation-neutral archive value model and how that value is reconstructed safely.

## Requirements

Preserve normal C++ value semantics, checked numeric conversion, optional/collection ownership, detailed failures, schema/default behaviour, and compatibility across all archives that support the required structural shape.

## Collections

When adding container support, account for element limits and allocation growth. Do not make an unbounded container an implicit bypass around archive decode limits.

## Custom types

Prefer adapting a stable value abstraction or making a complex model Serializable itself rather than serializing opaque memory layout.

## Testing

Exercise value composition as members, nested values, standard-container elements, copy/move construction, invalid input, and multiple archive representations.