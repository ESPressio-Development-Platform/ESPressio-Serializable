# ESPressio Serializable

Declarative, representation-neutral serialization components for the ESPressio Development Platform.

ESPressio Serializable lets a C++ type describe **which members constitute its serializable state** without hard-coding whether that state becomes JSON, CBOR, ESPB Binary, a stream, persisted data, or another representation.

## Latest Stable Version

**0.11.0**

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

# Installation

Core only:

```ini
lib_deps =
    espressio-development-platform/ESPressio-Serializable@^0.11.0
```

Protected serialization additionally requires ESPressio Security 0.4.x:

```ini
lib_deps =
    espressio-development-platform/ESPressio-Serializable@^0.11.0
    espressio-development-platform/ESPressio-Security@^0.4.0
    espressio-development-platform/ESPressio-Observable@^3.0.2
```

Use the core umbrella normally:

```cpp
#include <ESPressio_Serializable.hpp>
```

Include Security integration only where required:

```cpp
#include <ESPressio_Serializable_Security.hpp>
```

# Declaring a Serializable type

```cpp
#include <ESPressio_Serializable.hpp>

class DeviceConfiguration final
    : public ESPressio::Serializable::Serializable<DeviceConfiguration> {

    ESPRESSIO_SERIALIZABLE_TYPE(DeviceConfiguration)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)

private:
    uint32_t _sampleRate = 1000;
    float _threshold = 0.5f;
    bool _loggingEnabled = true;

public:
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY("sampleRate", _sampleRate),
        ESPRESSIO_PROPERTY("threshold", _threshold),
        ESPRESSIO_PROPERTY("loggingEnabled", _loggingEnabled)
    )
};
```

The declaration describes the model, not its representation or destination.

# Protected serialization — quickest path

Protection is applied to the **complete finished archive**, not to individual fields. This means schema migration/defaults/aliases still operate on the normal archive after successful authentication/decryption.

First configure Security once:

```cpp
#include <array>
#include <ESPressio_Security.hpp>
#include <ESPressio_Serializable_Security.hpp>

using namespace ESPressio;

Security::AES256GCMCipher cipher;
Security::AeadCipherRegistry ciphers;
Security::StaticKeyProvider keys;
Security::ESP32RandomSource randomSource;

constexpr std::array<uint8_t, 32> ApplicationKey = {
    /* application-specific bytes */
};

ciphers.Register(cipher);
keys.Add(1, Security::AeadAlgorithm::AES256GCM, ApplicationKey);

Security::DataProtectionConfig dataProtection;
dataProtection.Algorithm = Security::AeadAlgorithm::AES256GCM;
dataProtection.KeyID = 1;

Security::DataProtector protector(
    ciphers,
    keys,
    randomSource,
    dataProtection
);

Serializable::SerializationProtectionConfig protection(
    protector,
    "MyApplication.DeviceConfiguration"
);
```

Then protecting a Serializable object is one call:

```cpp
DeviceConfiguration configuration;
std::vector<uint8_t> protectedBytes;

auto result = Serializable::SerializeProtectedBinary(
    configuration,
    protectedBytes,
    protection
);

if (!result) {
    // result.Status
    // result.SecurityResult
}
```

And restoring directly into the applicable concrete type is symmetrical:

```cpp
DeviceConfiguration restored;

auto result = Serializable::DeserializeProtectedBinary(
    protectedBytes.data(),
    protectedBytes.size(),
    restored,
    protection
);

if (!result) {
    // Security failure is kept separate from schema/deserialization issues.
}
```

The complete Arduino example is in:

```text
examples/ProtectedBinary/
```

## Why `SerializationProtectionConfig`?

The configuration keeps cryptographic mechanics out of model and application call-sites. It contains:

- the configured `Security::IDataProtector` implementation;
- an authenticated purpose/context string;
- maximum permitted archive size;
- BinaryArchive decode limits; and
- detailed deserialization options.

Higher-level libraries can therefore supply sensible context values while developers only provide the protector/key policy.

## Purpose/context binding

The context is authenticated but not stored in the protected representation:

```cpp
Serializable::SerializationProtectionConfig protection(
    protector,
    "ESPressio.WiFi.Configuration"
);
```

A protected blob produced for that purpose cannot be successfully restored using a different context. This is useful when the same key provider protects multiple unrelated persisted records.

## Protection is optional

The configuration-driven helpers accept a nullable protection pointer:

```cpp
std::vector<uint8_t> bytes;

Serializable::SerializeBinary(
    configuration,
    bytes,
    &protection       // protected
);

Serializable::SerializeBinary(
    configuration,
    bytes,
    nullptr           // ordinary ESPB
);
```

This makes it straightforward for a higher-level library to expose protection as an opt-in feature without maintaining two completely different serialization paths.

# Redaction is not encryption

Serializable redaction metadata and authenticated encryption solve different problems:

```text
Redaction
    prevents sensitive values being emitted in diagnostics/operator views

Protection
    protects the complete serialized representation at rest/in an opaque byte channel
```

A password or secret field should still be marked redacted even when the stored configuration is encrypted. Conversely, redaction does **not** make persisted bytes confidential.

# Protection result model

`ProtectedSerializationResult` retains distinct layers of failure information:

```text
ProtectedSerializationStatus
SecurityResult
DeserializationResult
ArchiveBytes
ProtectedBytes
```

Callers can therefore distinguish authentication/key failures from malformed ESPB, migration failures, missing required properties or validation errors.

# BinaryArchive and schema evolution

Protected serialization initially targets the normal ESPB `BinaryArchive` rather than the direct same-schema fast path because persisted data frequently outlives the firmware that wrote it.

The order is:

```text
object
  -> BinaryArchive / ESPB
  -> Security::IDataProtector
  -> protected bytes

protected bytes
  -> Security::IDataProtector
  -> BinaryArchive / ESPB
  -> migrations/defaults/aliases/validation
  -> object
```

The ESPB v2 representation itself is unchanged by 0.11.0; protection wraps it externally.

# JSON, CBOR and other representations

The ordinary representation families remain available:

- JSON for human-readable/interoperable interfaces;
- CBOR for compact structured interchange;
- ESPB Binary for ESPressio-native representation and persistence;
- direct ESPB Binary for latency-sensitive same-schema paths;
- streaming facilities for larger data sets;
- custom archives for application-specific representations.

Representative examples include:

```text
examples/JsonArchive/
examples/CborArchive/
examples/BinaryArchive/
examples/ProtectedBinary/
examples/CustomArchive/
examples/SerialCborTransport/
```

# Deserialization diagnostics

Detailed deserialization can report:

- missing required properties;
- invalid value types;
- numeric constraint failures;
- unknown enum mappings;
- schema/migration failures; and
- application-defined validation failures.

Use `DeserializeDetailed()` or inspect `ProtectedSerializationResult::Deserialization` rather than collapsing all failures into a boolean.

# Nested values and collections

Serializable supports nested objects and common collection/value forms including arithmetic values, strings/Arduino `String`, arrays/containers, `std::optional`, enums, maps and sets where supported by the selected archive.

See:

```text
examples/NestedCollections/
examples/OptionalAndMaps/
examples/EnumMapping/
```

# Schema evolution

Persisted data often outlives firmware. Schema versions, aliases, defaults and migration helpers are therefore first-class facilities:

```text
old representation
      |
      v
migration / aliases / defaults
      |
      v
current schema
```

See `examples/SchemaEvolution/`.

# Bounded Binary decoding

`BinaryArchiveDecodeLimits` constrains nesting depth, total nodes, object members, array elements, property-name lengths and string lengths. Protected serialization carries these limits in `SerializationProtectionConfig`, so authentication does not remove the need for bounded decoding.

# Direct Binary fast path

For latency-sensitive same-schema use:

```cpp
std::vector<uint8_t> bytes;
Serializable::SerializeDirectBinary(object, bytes);
Serializable::DeserializeDirectBinary(bytes.data(), bytes.size(), object);
```

The direct path preserves ESPB v2 byte compatibility but is not the default protected-persistence path because the tree/archive path provides structural migration facilities.

# Dependencies

Core Serializable 0.11.0 still has **no required ESPressio dependency**.

```text
Serializable core
    -> none

Serializable Security integration
    - - -> Security >= 0.4.0 < 1.0.0
```

Downstream libraries such as Persistence, Event and Units may opt into Serializable independently.

See [ESPRESSIO_DEPENDENCY_CHART.md](ESPRESSIO_DEPENDENCY_CHART.md).

# Design principles

- One authoritative declarative schema per type.
- Model classes own state; archives own representation.
- Protection wraps representations rather than contaminating model schemas with cryptographic concerns.
- Redaction and encryption remain deliberately separate.
- Embedded-friendly bounded decoding and resource limits.
- Detailed errors rather than generic parse failures.
- Optional integrations must not turn foundational dependencies into mandatory ones.

# Changelog

See [CHANGELOG.md](CHANGELOG.md).

## License

Apache License 2.0. See [LICENSE](LICENSE).
