# CBOR Archives

CBOR provides compact structured interchange while preserving the same declarative Serializable schema used by JSON and ESPB representations.

Use CBOR when a structured, self-describing binary representation is useful but JSON's textual overhead is undesirable.

## Model independence

Serializable models do not contain CBOR-specific code. The archive maps declared property/value structures into CBOR and back.

## Transport use

CBOR is suitable for machine-oriented transports and streaming integrations where both endpoints understand the representation.

## Resource safety

Compact encoding does not remove the need for bounded decoding. Apply appropriate limits to nesting, collections, strings, and input size at trust boundaries.