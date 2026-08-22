# ESPressio Serializable

Declarative, representation-neutral serialization components for the ESPressio Development Platform.

ESPressio Serializable lets a C++ type answer one question—**which members constitute my serializable state?**—without hard-coding whether that state will become JSON, CBOR, ESPB binary, NVS, a stream, or another representation.

## Latest Stable Version

**0.10.2**

The 0.10.x line adds the direct ESPB Binary fast path and bounded/allocation-free BinaryArchive validation/traversal used by latency- and memory-sensitive integrations. 0.10.2 itself is warning-clean maintenance and does not change the wire format.

# Why not just write `ToJson()` / `FromJson()`?

Doing so couples the model to JSON.

If the same state later needs to be stored in NVS, sent as CBOR, carried over Event Transport, inspected diagnostically, or encoded as compact binary, the object either gains more representation-specific responsibilities or requires parallel conversion code.

ESPressio Serializable separates the concerns:

```text
Serializable object
        |
        v
Declarative property/schema metadata
        |
        v
Representation-neutral serialization model
        |
        +---- JSON
        +---- CBOR
        +---- ESPB Binary
        +---- NVS
        +---- streaming/custom archive
```

The object's declaration remains the authoritative schema.

# Compatibility

The core has no required ESPressio dependency and no ESP32-specific runtime requirement. It uses C++17 language/library facilities and is designed for Arduino/PlatformIO targets as well as host-side testing where the selected archive dependencies are available.

Representation-specific adapters may introduce their own dependencies (for example ArduinoJson for JSON).

# ESPressio Development Platform

ESPressio libraries are designed to be light-weight, composable, strongly typed, object-oriented, and explicit about dependency direction. Serializable is intentionally foundational: other libraries opt into it; Serializable does not depend back on them.

Project website: [espressio.org](https://espressio.org).

## License

Apache License 2.0. See [LICENSE](LICENSE).

# Namespace

```cpp
ESPressio::Serializable
```

Important concepts include:

- `Serializable<T>` / `SerializableBase<T>`
- declarative property/schema macros
- `SerializationNode`
- JSON, CBOR and Binary archives
- direct Binary serialization/deserialization
- streaming archive facilities
- schema introspection/evolution
- validation and diagnostics
- enum mapping
- redaction metadata
- bounded BinaryArchive validation/traversal

# Dependencies

Required ESPressio dependencies: **none**.

Other ESPressio libraries opt into Serializable where appropriate:

```text
Units
    - - -> Serializable Unit variants

Event
    - - -> Serializable Events / Event Transport
```

See [ESPRESSIO_DEPENDENCY_CHART.md](ESPRESSIO_DEPENDENCY_CHART.md).

# Installation

Core:

```ini
lib_deps =
    espressio-development-platform/ESPressio-Serializable@^0.10.2
```

Select the archive-specific headers/dependencies required by the application rather than assuming JSON is mandatory.

# Declaring a Serializable type

A type declares its serializable contract alongside its state:

```cpp
#include <ESPressio_Serializable.hpp>

class DeviceConfiguration final :
    public ESPressio::Serializable::
        Serializable<DeviceConfiguration> {

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

The property declarations describe **what** belongs to the schema. They do not decide the representation.

# Basic JSON example

JSON is an adapter over the same schema:

```cpp
#include <Arduino.h>
#include <ESPressio_Serializable_JSON.hpp>

using namespace ESPressio;

class Settings final :
    public Serializable::Serializable<Settings> {

    ESPRESSIO_SERIALIZABLE_TYPE(Settings)

private:
    uint32_t _sampleRate = 1000;
    float _threshold = 0.5f;
    bool _enabled = true;

public:
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY("sampleRate", _sampleRate),
        ESPRESSIO_PROPERTY("threshold", _threshold),
        ESPRESSIO_PROPERTY("enabled", _enabled)
    )
};

void setup() {
    Serial.begin(115200);

    Settings settings;
    Serializable::JsonArchive archive;

    settings.Serialize(archive);
    archive.SavePretty(Serial);
    Serial.println();
}
```

The complete current example is under:

```text
examples/JsonArchive/
```

The same `Settings` declaration can be consumed by another archive without adding JSON-specific methods to `Settings`.

# Deserialization and diagnostics

Deserialization uses the same declared schema in the opposite direction. Detailed deserialization can report issues such as:

- missing required properties;
- invalid value types;
- numeric constraint failures;
- unknown enum mappings;
- schema/migration problems; and
- application-defined validation failures.

Applications should surface those structured diagnostics rather than reducing every failure to a generic “parse failed” message.

See:

```text
examples/DeserializeConfiguration/
```

# Representation-neutral `SerializationNode`

`SerializationNode` is the tree representation used when a caller needs to inspect/manipulate structured serialized data without committing the model itself to JSON/CBOR/etc.

This is especially useful at integration boundaries:

```text
JSON / CBOR / replay / CLI
          |
          v
   SerializationNode
          |
          v
Serializable object
```

ESPressio Event runtime Serializable Event construction uses this model so Event remains independent of ArduinoJson.

# JSON, CBOR and Binary representations

The library supports multiple representation families:

- JSON for human-readable/interoperable configuration and operator interfaces;
- CBOR for compact structured interchange;
- ESPB Binary for compact ESPressio-native payloads and Event Transport;
- NVS integration for embedded persistence; and
- custom/streaming archives where the application has another representation.

Current examples include:

```text
examples/JsonArchive/
examples/CborArchive/
examples/BinaryArchive/
examples/NvsArchive/
examples/CustomArchive/
examples/SerialCborTransport/
```

# Nested objects and collections

Nested Serializable objects and common collection forms are supported so a schema can reflect the natural shape of application state rather than flattening everything into primitive fields.

Representative examples:

```text
examples/NestedCollections/
examples/OptionalAndMaps/
```

The value layer includes arithmetic/scalar types, Arduino `String`, nested Serializable values, supported arrays/containers, `std::optional`, enum mappings and supported map/set forms.

# Enum mapping

Enums can be mapped deliberately between C++ values and stable serialized names rather than leaking underlying integer values into a public schema.

See:

```text
examples/EnumMapping/
```

This makes wire/configuration values readable and provides a controlled failure mode for unknown enum names.

# Validation

Properties and whole objects can participate in validation.

Use declarative constraints for schema-level rules (for example numeric ranges) and application-defined validation callbacks for invariants that require custom logic.

The same validation rules apply regardless of whether the representation arrived through JSON, CBOR, Binary, Event Transport, or another compatible archive.

# Schema evolution

Persisted/transported data often outlives the firmware that produced it. Serializable therefore treats schema versioning/evolution as a first-class concern rather than assuming every stored payload matches today's C++ layout.

Schema facilities include version declarations, aliases, defaults and migration helpers.

Conceptually:

```text
old representation
      |
      v
schema migration / aliases / defaults
      |
      v
current object schema
```

See:

```text
examples/SchemaEvolution/
```

# Schema introspection

Serializable metadata can be inspected without manually duplicating property descriptions in operator tooling.

This enables consumers such as EventConsole to describe runtime Event payload schemas from the same declarations used for actual serialization/validation.

See:

```text
examples/SchemaIntrospection/
```

# Redaction

Schema metadata can mark fields that should not be emitted plainly in diagnostic/log-oriented representations.

Redaction is a documentation/diagnostics safety facility; it should not be confused with encryption or secure storage. Secrets still require an appropriate Security/storage design.

# Direct Binary fast path

Latency-sensitive integrations can serialize directly into the ESPB v2 wire representation without constructing a complete intermediate `SerializationNode` tree:

```cpp
std::vector<uint8_t> bytes;

ESPressio::Serializable::SerializeDirectBinary(
    object,
    bytes
);

ESPressio::Serializable::DeserializeDirectBinary(
    bytes.data(),
    bytes.size(),
    object
);
```

`AppendDirectBinary()` is useful when a caller has already written/reserved a protocol prefix and wants the Serializable payload appended to the same final buffer.

The fast path preserves the existing ESPB v2 wire format. Consumers that require structural schema migration can continue to use the tree/archive path; Event Transport can fall back accordingly.

# Bounded Binary decoding

Untrusted Binary input must not be allowed to request unbounded recursive allocation. `BinaryArchiveDecodeLimits` controls embedded-friendly bounds for:

- nesting depth;
- aggregate node count;
- object members;
- array elements;
- property-name length; and
- string length.

```cpp
ESPressio::Serializable::BinaryArchive archive;
ESPressio::Serializable::BinaryArchiveDecodeLimits limits;

limits.MaximumDepth = 16;
limits.MaximumTotalNodes = 1024;
limits.MaximumObjectMembers = 256;
limits.MaximumArrayElements = 1024;
limits.MaximumNameLength = 256;
limits.MaximumStringLength = 16 * 1024;

if (!archive.Load(data, size, limits)) {
    // malformed, truncated, unsupported, or outside configured limits
}
```

The no-options overload remains source-compatible and applies library defaults.

# Allocation-free Binary validation and traversal

Diagnostics and protocol inspection often need to validate/display a Binary payload without building another heap-backed object tree.

```cpp
ESPressio::Serializable::BinaryArchiveDecodeLimits limits;
limits.MaximumDepth = 12;
limits.MaximumTotalNodes = 1024;

if (ESPressio::Serializable::ValidateBinaryArchive(
        data,
        size,
        limits
    )) {
    // structurally valid and within configured limits
}
```

`TraverseBinaryArchive()` streams object/array/property/scalar callbacks through a `BinaryArchiveVisitor`, borrowing names/strings via `std::string_view` instead of copying them into a second tree.

This is the facility used by ESPressio Serial's hardened EventMonitor diagnostics so low-memory diagnostics do not create another avoidable allocation spike.

# Streaming

Streaming readers/writers exist where supported by an archive so large payloads do not always need to be materialized as one complete in-memory representation.

See:

```text
examples/LargeJsonStream/
examples/SerialCborTransport/
```

for application-shaped usage.

# Nameless/compact Binary use

Binary representations can take advantage of schema-known ordering/name elimination when the selected mode permits it, reducing overhead for tightly controlled embedded protocols.

See:

```text
examples/NamelessBinary/
```

# Compile-time diagnostics

Where an invalid/unsupported schema declaration can be diagnosed statically, Serializable aims to fail at compile time instead of allowing an unsupported member to fail only after deployment.

This is particularly valuable in a declarative schema system: the compiler becomes part of schema validation.

# Custom archives

The archive abstraction remains an extension point. Applications can define representation-specific behavior without changing each Serializable model class.

The repository maintains:

```text
examples/CustomArchive/
```

as the current reference for implementing that extension against the current archive contract.

# Examples

The repository includes a broad executable example set, including:

```text
BasicSerialization
DeserializeConfiguration
JsonArchive
CborArchive
BinaryArchive
NvsArchive
CustomArchive
NestedCollections
OptionalAndMaps
EnumMapping
SchemaEvolution
SchemaIntrospection
LargeJsonStream
NamelessBinary
SerialCborTransport
ESPNowSerializableMesh
```

Use these current examples rather than old 0.1-era snippets when an archive's API has evolved.

# Design principles

- One authoritative declarative schema per type.
- Representation-neutral model metadata.
- Archives own representation; model classes own state.
- Embedded-friendly bounded decoding.
- Direct/streaming paths where intermediate trees are unnecessary.
- Validation and schema evolution are part of the schema contract.
- Compile-time diagnostics where possible.
- Serializable remains foundational and independent of downstream ESPressio libraries.

# Changelog

See [CHANGELOG.md](CHANGELOG.md) for release history and notable changes.
