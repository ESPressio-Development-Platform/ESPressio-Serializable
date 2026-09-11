# ESPressio Serializable

Declarative, representation-neutral serialization components for the ESPressio Development Platform.

ESPressio Serializable lets a C++ type describe **which members constitute its serializable state** without hard-coding whether that state becomes JSON, CBOR, ESPB Binary, a stream, persisted data, or another representation.

## Latest Stable Version

**0.11.2**

line adds an optional ESPressio Security integration for protecting complete serialized representations with authenticated encryption. Core Serializable remains Security-independent.

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

## Bounded Primitive serialization

The `primitives_redesign` branch derives both general archive behavior and bounded Primitive metadata from `GetSerializableProperties()`. `IsSerializable<T>` still accepts general objects containing `std::string`, vectors and other dynamic fields. `IsBoundedSerializable<T>` additionally requires an explicit nonzero schema version, a finite property graph, unambiguous names/aliases and no tree-only serialization adapter.

Property declarations no longer allocate a lazy shared tuple. Their builders and getters support constant evaluation; bounded schema declarations must be constant-evaluable. A bounded schema retains one constexpr immutable static property tuple and descriptor graph, with no first-use initialization lock. General types with dynamic defaults can still evaluate their declarations at runtime. `Required()`, `ReadOnly()`, `Sensitive()`, `Alias()`, `Default()`, `Range()` and `Validate()` remain the single metadata source. `IsMetadataValid()` detects alias requests that exceed the declared capacity. `HasMinimum()/GetMinimum()` and `HasMaximum()/GetMaximum()` expose explicit range metadata.

```cpp
#include <ESPressio_Serializable.hpp>
#include <array>
#include <cassert>
using namespace ESPressio::Serializable;

struct Reading : Serializable<Reading> {
    BoundedString<24> Label;
    BoundedVector<std::int16_t, 4> Samples;
    ESPRESSIO_SERIALIZABLE_TYPE(Reading)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY("label", Label).Required().Sensitive(),
        ESPRESSIO_PROPERTY("samples", Samples))
};
static_assert(IsSerializable<Reading> && IsBoundedSerializable<Reading>);
static_assert(SerializationTraits<Reading>::MinimumReadableVersion == 1);

void boundedRoundTrip() {
    Reading reading;
    assert(reading.Label.assign("room sensor"));
    assert(reading.Samples.push_back(21));
    std::array<std::uint8_t, MaximumSerializedSize<Reading, DirectBinary>> binary{};
    std::array<std::uint8_t, MaximumSerializedSize<Reading, CBOR>> cbor{};
    std::array<std::uint8_t, MaximumSerializedSize<Reading, JSON>> json{};
    auto b = SerializeDirectBinary(reading, binary.data(), binary.size());
    auto c = SerializeBoundedCbor(reading, cbor.data(), cbor.size());
    auto j = SerializeBoundedJson(reading, json.data(), json.size());
    assert(b && c && j);
    Reading restored;
    assert(DeserializeBoundedDirectBinary(binary.data(), b.Bytes, restored));
    assert(DeserializeBoundedCbor(cbor.data(), c.Bytes, restored));
    assert(DeserializeBoundedJson(json.data(), j.Bytes, restored));
    const auto& schema = SchemaDescriptor<Reading>();
    assert(schema.CurrentVersion == 1 && schema.PropertyCount == 2);
    assert(schema.Properties[0].Name == "label");
    assert(schema.Properties[0].Value->Cardinality == 24);
    assert(schema.MaximumDirectBinaryBytes == binary.size());
    // StructuralFingerprint is portable semantic metadata for the owning family contract.
    assert(schema.StructuralFingerprint != 0);
    const auto properties = Reading::GetSerializableProperties();
    const auto& label = std::get<0>(properties);
    assert(label.IsMetadataValid() && !label.HasMinimum() && !label.HasMaximum());
    static_assert(IsBoundedWireValue<BoundedString<24>>);
}
```

`WriteCanonicalSchema<T>(sink)` streams the full normalized schema to a family-owned bounded sink. The sink supplies `Byte`, eight-byte little-endian `Integer`, and length-prefixed `Text` operations, all `noexcept`. Families use this stream directly in their full contract fingerprint; they must not expand the 64-bit `StructuralFingerprint` into a supposedly stronger digest. The stream includes the versioned format domain, sorted properties and aliases, value shapes, enum mappings, defaults and ranges. It never includes member addresses or compiler Type names.

`BoundedSerializationResult` contains an error code and actual byte count. Its boolean conversion means success. Failed decoding leaves the destination unchanged; failed encoding leaves unpublished scratch bytes and reports zero bytes. The caller publishes a buffer only after success. No presentation redaction/omission policy is accepted by canonical codecs. Canonical reconstruction hydrates all semantic fields, including fields marked ReadOnly for general archive/tooling mutation. General archive deserialization retains its ReadOnly behavior.

`MaximumSerializedSize<T, Format>` covers the complete object, including ESPB framing where applicable, explicit schema version, names (including longer accepted aliases), count/length/type tags, nested structures and worst-case JSON escaping. The owning family adds its fixed wire envelope; Radio/Mesh/security add their own overhead separately. Arithmetic overflow in a bound fails compilation. JSON emits compact round-trippable numbers, rejects nonfinite floating point and invalid UTF-8, and has no ArduinoJson dependency on its bounded path.

Containers expose fixed inline storage and report capacity failure without growing:

```cpp
#include <ESPressio_BoundedContainers.hpp>
#include <cassert>
using namespace ESPressio::Serializable;
void boundedContainers() {
    BoundedString<8> text;
    assert(text.assign("sensor") && text.push_back('1'));
    assert(text.size() == 7 && text.capacity() == 8);
    assert(text.view() == "sensor1" && text.c_str()[7] == 0);
    BoundedBytes<2> bytes;
    assert(bytes.push_back(1) && bytes.push_back(2) && !bytes.push_back(3));
    assert(bytes.data()[0] == 1 && bytes[1] == 2);
    for (auto byte : bytes) assert(byte != 0);
    BoundedSet<int, 2> values;
    assert(values.insert(7) && values.contains(7) && !values.insert(7));
    BoundedMap<int, bool, 2> flags;
    assert(flags.insert(7, true) && *flags.find(7));
    assert(flags.find(8) == nullptr);
    text.clear(); bytes.clear(); values.clear(); flags.clear();
    assert(text.empty() && bytes.empty() && values.size() == 0 && flags.size() == 0);
}
```

`BoundedVector<T,N>` and `BoundedBytes<N>` reserve all N value-initialized slots plus a count. `BoundedString<N>` reserves N+1 bytes and a count. Sets/maps use the same fixed sequence storage and linear lookup, with immutable iteration to preserve uniqueness. Duplicate set/map insertion fails. Clearing resets active slots without changing capacity. Element constructors/assignments retain their own C++ exception contract; Primitive qualification checks their serialized graph and does not certify opaque application code.

Arrays, optional values, variants, enums and nested bounded schemas participate recursively. Variants encode an `index` and `value` object. A present optional containing another optional uses a one-field `value` object, so outer absence and inner absence remain distinct. There is no predecessor-format fallback. Older schema versions are not accepted by bounded codecs until a complete bounded migration path is provided; the general tree migration helpers remain available for general serialization.

`SchemaDescriptor<T>()` exposes immutable properties, aliases, flags, kinds, cardinalities, per-format bounds, nested schemas and variant alternatives. Fingerprints include schema/readable versions, normalized types, bounds, aliases, defaults and range/required/read-only/sensitive semantics, and enum mappings. They exclude object layout, compiler names, member addresses, tuple declaration order and opaque validator addresses. Tree-backed SchemaInspector presentation helpers remain general conveniences; Primitive directory/binding consumers use the static descriptor.

See [bounded validation and resource accounting](docs/BOUNDED_SERIALIZATION.md) for test classification and memory formulas.

# Installation

Core only:

```ini
lib_deps =
    https://github.com/ESPressio-Development-Platform/ESPressio-Serializable.git#primitives_redesign
```

Protected serialization additionally requires ESPressio Security from `primitives_redesign`:

```ini
lib_deps =
    https://github.com/ESPressio-Development-Platform/ESPressio-Serializable.git#primitives_redesign
    https://github.com/ESPressio-Development-Platform/ESPressio-Security.git#primitives_redesign
    https://github.com/ESPressio-Development-Platform/ESPressio-Observable.git#primitives_redesign
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

The ESPB v2 representation itself is unchanged by 0.11.2; protection wraps it externally.

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

Core Serializable still has **no required ESPressio dependency**.

```text
Serializable core
    -> none

Serializable Security integration
    - - -> Security main
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



## License

Apache License 2.0. See [LICENSE](LICENSE).
