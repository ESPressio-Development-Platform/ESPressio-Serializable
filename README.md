# ESPressio Serializable

Declarative, representation-neutral serialization components for the ESPressio Development Platform.

ESPressio Serializable lets a C++ type describe **which members constitute its serializable state** without hard-coding whether that state becomes JSON, CBOR, ESPB Binary, a stream, persisted data, or another representation.

## Latest Stable Version

**0.11.2**

The 0.11.x line adds an optional ESPressio Security integration for protecting complete serialized representations with authenticated encryption. Core Serializable remains Security-independent.

# Why representation-neutral serialization?

A model should not need `ToJson()`, `ToCbor()`, `ToBinary()`, `SaveToNVS()` and similar parallel implementations.

```text
Serializable object
        |
        v
Declarative schema
        |
        v
Representation-neutral archives
        |
        +---- JSON
        +---- CBOR
        +---- ESPB Binary
        +---- direct ESPB Binary
        +---- streaming/custom archives
        |
        +---- optional authenticated protection
```

The object's declaration remains authoritative.
