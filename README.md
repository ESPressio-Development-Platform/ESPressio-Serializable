# ESPressio Serializable

## Pre-release Status

> **Version 0.9.0 — API Freeze / Downstream Validation**

This release is intended to be tagged and consumed by downstream projects before `1.0.0`. The implementation is considered feature-complete for the initial stable release, but hardware validation, integration testing, benchmarking, fuzzing, and final API review are still in progress.

Breaking API changes remain possible during the `0.9.x` line if downstream testing exposes a release-blocking design problem. The goal is to make those corrections now so that `1.0.0` can establish a stable compatibility contract.


Serialization and Deserialization Components of the Flowduino ESPressio Development Platform

Provides a foundation for declaratively describing the serializable state of C++ types and serializing or deserializing that state through interchangeable Archive implementations.

The core library deliberately does not prescribe JSON, CBOR, NVS, filesystem, network, or other storage/transport representations. Instead, implementing classes expose their serializable members once, while Archive implementations determine how those members are represented.

## Latest Stable Version

ESPressio Serializable is currently in **pre-release API-freeze validation**.

The current pre-release is version `0.9.0`.

Version `0.9.0` is intended for downstream integration, hardware validation, compatibility testing, and final API review before the first stable `1.0.0` release.

No new feature work is planned for the `0.9.x` line unless it is required to correct a design flaw, compatibility issue, or release-blocking defect. The focus of this phase is:

* validating the public API from real consuming projects;
* compiling and running on representative ESP32 targets;
* verifying PlatformIO and Arduino IDE compatibility;
* exercising JSON, CBOR, Binary, NVS, streaming, and ESP-NOW paths on hardware;
* extending fuzzing and malformed-input coverage;
* confirming wire-format compatibility guarantees;
* measuring RAM, flash, and runtime characteristics;
* refining documentation and examples; and
* resolving any issues discovered before `1.0.0`.

The public API should now be treated as **provisionally frozen**. Any breaking change discovered during `0.9.x` validation should be made before `1.0.0`, not deferred until after it.

## Compatibility

ESPressio Serializable is designed primarily for use with the Arduino framework and PlatformIO, including ESP32 targets used by the wider ESPressio Development Platform.

Unlike several other ESPressio Component Libraries, the core Serializable implementation has no ESP32-specific dependency and contains no direct FreeRTOS or ESP-IDF functionality.

The core implementation requires a C++17-capable toolchain for features including `std::apply`, fold expressions, `std::void_t`, and inline variable templates.

Compatibility should be verified by compiling for the intended board/core/toolchain combination.

## ESPressio Development Platform

The ESPressio Development Platform is a collection of discrete (sometimes intra-connected) Component Libraries developed with a particular development ethos in mind.

The key objectives of the ESPressio Development Platform are:

* Light-weight - The Components should always strive to optimize memory consumption and operational overhead as much as possible, but not to the detriment of...
* Ease of Use - Many of our components serve as Developer-Friendly Abstractions of existing procedural code libraries.
* Object-Oriented - A `type` for everything, and everything in a `type`!
* SOLID:
    * > **Single Responsibility Principle (SRP)** Break your code into smaller, focused components.
    * > **Open/Closed Principle (OCP)** Be open for extension but closed for modification.
    * > **Liskov Substitution Principle (LSP)** Be substitutable for the base type without altering correctness.
    * > **Interface Segregation Principle (ISP)** Break interfaces into specific, client-focused ones.
    * > **Dependency Inversion Principle (DIP)** Be dependent on abstractions, not concretions.

To the maximum extent possible within the limitations/restrictions/constraints of the C++ language, the Arduino platform, and Microcontroller Programming itself, all Component Libraries of the ESPressio Development Platform must strive to honour the SOLID principles.

## License

ESPressio (and its component libraries, including this one) are subject to the Apache License 2.0. Please see the accompanying `LICENSE` file for full details.

## Namespace

Every type/variable/constant/etc. related to ESPressio Serializable is located within the `Serializable` sub-namespace of the `ESPressio` parent namespace.

For example:

```cpp
ESPressio::Serializable::Serializable<T>
ESPressio::Serializable::SerializableBase<T>
ESPressio::Serializable::SerializationProperty<TObject, TValue>
ESPressio::Serializable::KeyValueArchive
```

The namespace currently provides the following principal components:

* `ESPressio::Serializable::Serializable<T>`
* `ESPressio::Serializable::SerializableBase<T>`
* `ESPressio::Serializable::SerializationProperty<TObject, TValue>`
* `ESPressio::Serializable::KeyValueArchive`
* `ESPressio::Serializable::IsSerializable<T>`

It also provides the following declaration macros:

* `ESPRESSIO_SERIALIZABLE_TYPE(Type)`
* `ESPRESSIO_SERIALIZABLE_PROPERTIES(...)`
* `ESPRESSIO_PROPERTY(Name, Member)`

## Dependencies

The ESPressio Serializable core library has no dependencies on other ESPressio Component Libraries.

It also deliberately has no dependency on ArduinoJson, CBOR libraries, ESP32 Preferences/NVS, filesystem libraries, or any particular serialization representation.

Representation-specific support is intended to be provided through Archive implementations.

This keeps the fundamental Serializable abstraction portable and prevents implementing classes from becoming coupled to the mechanism by which their state is persisted or transported.

## Platformio.ini

You can add a local development checkout to a PlatformIO project using an appropriate `lib_deps` path.

Once the library is published to the PlatformIO Registry, the intended form will be:

```ini
lib_deps =
    flowduino/ESPressio-Serializable@^0.9.0
```

Alternatively, once the GitHub repository is public, the latest development sources can be referenced directly:

```ini
lib_deps =
    https://github.com/Flowduino/ESPressio-Serializable.git
```

Please note that using the GitHub repository directly will use the latest commits pushed into the repository, so volatility is possible.

## Optional Archive Implementations

The core umbrella header:

```cpp
#include <ESPressio_Serializable.hpp>
```

includes **only the core Serializable implementation**.

None of the optional representation or transport adapters are included implicitly.

A consuming project opts into only the functionality it needs by including the corresponding adapter header.

### Native Binary

```cpp
#include <ESPressio_Serializable_Binary.hpp>
```

Provides:

```cpp
ESPressio::Serializable::BinaryArchive
```

The Binary Archive uses a compact ESPressio-specific, versioned binary key/value representation and has no dependency on Arduino or any third-party library.

### CBOR

```cpp
#include <ESPressio_Serializable_CBOR.hpp>
```

Provides:

```cpp
ESPressio::Serializable::CborArchive
```

The initial CBOR Archive is self-contained and emits standards-compatible CBOR maps containing primitive values and UTF-8 strings.

It does not require an external CBOR library.

### JSON

```cpp
#include <ESPressio_Serializable_JSON.hpp>
```

Provides:

```cpp
ESPressio::Serializable::JsonArchive
```

JSON support is implemented as an adapter over ArduinoJson 7.

The ESPressio library itself does **not** declare ArduinoJson as an unconditional dependency. A project which chooses JSON support should add ArduinoJson to its own PlatformIO dependencies:

```ini
lib_deps =
    flowduino/ESPressio-Serializable@^0.9.0
    bblanchon/ArduinoJson
```

A project which does not include `ESPressio_Serializable_JSON.hpp` does not compile the JSON adapter and does not need ArduinoJson.

### Arduino Stream Transport

```cpp
#include <ESPressio_Serializable_Stream.hpp>
```

Provides:

```cpp
ESPressio::Serializable::StreamArchive
```

`StreamArchive` is deliberately considered a **transport adapter rather than a representation**.

It transports the payload produced by another Archive using the Arduino `Stream` abstraction. This permits the same serialized representation to be moved through implementations such as:

* `HardwareSerial`
* `WiFiClient`
* `File`
* other Arduino `Stream` derivatives

For example:

```cpp
Serializable::StreamArchive::Serialize<
    Serializable::BinaryArchive
>(
    object,
    Serial
);
```

The stream transport uses a 32-bit little-endian payload length prefix so that multiple objects can safely share a continuous stream.

Because this adapter uses Arduino `Stream`, the header is available only when the Arduino framework is present.

### Why the Optional Headers are Separate

This separation is intentional:

```text
ESPressio_Serializable.hpp
        |
        +---- Core only
        |
        +---- no ArduinoJson
        +---- no Arduino Stream
        +---- no CBOR dependency
        +---- no format-specific code
```

The consuming project explicitly opts into additional functionality:

```text
ESPressio_Serializable_Binary.hpp
        |
        +---- Core
        +---- BinaryArchive

ESPressio_Serializable_CBOR.hpp
        |
        +---- Core
        +---- CborArchive

ESPressio_Serializable_JSON.hpp
        |
        +---- Core
        +---- JsonArchive
        +---- ArduinoJson (consumer-selected dependency)

ESPressio_Serializable_Stream.hpp
        |
        +---- Core
        +---- StreamArchive
        +---- Arduino Stream
```

This means the absence of an optional header reference prevents that adapter from participating in the compilation unit.

In particular, including the main `ESPressio_Serializable.hpp` header will never silently pull JSON, CBOR, binary, or Arduino Stream functionality into the consuming application.

## What is ESPressio Serializable?

ESPressio Serializable provides a common mechanism by which a C++ type can describe the members that constitute its serializable state.

C++17 does not provide the runtime reflection or user-defined annotation facilities commonly available in languages such as C# or Java. ESPressio Serializable therefore provides a small compile-time metadata mechanism which allows an implementing class to declaratively identify its serializable members.

For example:

```cpp
class DeviceConfiguration final
    : public ESPressio::Serializable::Serializable<DeviceConfiguration> {

    ESPRESSIO_SERIALIZABLE_TYPE(DeviceConfiguration)

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

The implementing class describes **what** is serializable, but does not describe **how** that state is represented.

This distinction is fundamental to the design of the library.

A JSON Archive may serialize the object as JSON. An NVS Archive may persist the same properties into ESP32 Preferences. A CBOR Archive may encode the same object into a compact binary representation.

The implementing class remains unchanged.

## Why not simply implement `ToJson()` and `FromJson()`?

Doing so couples the implementing type directly to JSON.

If that same object later needs to be persisted to NVS, transferred as CBOR, written to a binary file, or represented in some other format, the object itself must either acquire additional responsibilities or its JSON-specific implementation must be worked around.

ESPressio Serializable instead applies the Dependency Inversion Principle:

```text
Serializable Object
        |
        v
Serializable Property Metadata
        |
        v
Archive Abstraction
        |
        +---- JSON
        |
        +---- CBOR
        |
        +---- NVS
        |
        +---- Binary
        |
        +---- Custom Representation
```

The object owns its state.

The property declarations describe that state.

The Archive determines its representation.

## Understanding the Components of ESPressio Serializable

Before looking at complete examples, it is useful to understand the individual components and their responsibilities.

### `Serializable<T>`

`Serializable<T>` is the normal base type from which a serializable class inherits.

It is currently an alias of `SerializableBase<T>`:

```cpp
template<typename TDerived>
using Serializable = SerializableBase<TDerived>;
```

The library uses the Curiously Recurring Template Pattern (CRTP), meaning that the implementing type passes itself as the template parameter:

```cpp
class Settings
    : public ESPressio::Serializable::Serializable<Settings> {
};
```

This allows serialization traversal to be resolved at compile time without requiring runtime type discovery.

### `SerializableBase<T>`

`SerializableBase<T>` implements the common `Serialize()` and `Deserialize()` traversal.

Calling:

```cpp
configuration.Serialize(archive);
```

enumerates the properties declared by the implementing type and invokes:

```cpp
archive.Write(propertyName, propertyValue);
```

for each property.

Likewise:

```cpp
configuration.Deserialize(archive);
```

invokes:

```cpp
archive.Read(propertyName, propertyValue);
```

for each property.

The Archive interface is intentionally duck-typed. An Archive does not need to inherit from a specific base class in the initial implementation; it simply needs to provide compatible `Read()` and/or `Write()` operations.

### `SerializationProperty<TObject, TValue>`

A `SerializationProperty` associates a serialized property name with a pointer-to-member.

For example:

```cpp
ESPRESSIO_PROPERTY("sampleRate", _sampleRate)
```

ultimately describes the relationship between:

```text
Serialized Name        C++ Member
---------------        ----------
sampleRate          -> _sampleRate
```

The descriptor itself does not store a copy of the member value.

Instead, it stores compile-time metadata through which the value can be obtained from a specific object instance.

### Serializable Declaration Macros

Three macros currently provide the annotation-like declaration syntax.

First, identify the implementing type:

```cpp
ESPRESSIO_SERIALIZABLE_TYPE(DeviceConfiguration)
```

Then declare its serializable properties:

```cpp
ESPRESSIO_SERIALIZABLE_PROPERTIES(
    ESPRESSIO_PROPERTY("sampleRate", _sampleRate),
    ESPRESSIO_PROPERTY("threshold", _threshold),
    ESPRESSIO_PROPERTY("loggingEnabled", _loggingEnabled)
)
```

The macros are deliberately restricted to schema declaration.

They do not create a runtime property registry and they do not determine the output representation.

### `IsSerializable<T>`

`IsSerializable<T>` is a compile-time trait which determines whether a type exposes the Serializable property contract.

For example:

```cpp
static_assert(
    ESPressio::Serializable::IsSerializable<DeviceConfiguration>
);
```

This will become increasingly useful as Archive implementations gain recursive support for nested serializable objects and collections.

### Archives

An Archive is responsible for converting between C++ values and some external representation.

For serialization, an Archive exposes compatible `Write()` operations:

```cpp
archive.Write("sampleRate", sampleRate);
archive.Write("enabled", enabled);
```

For deserialization, it exposes compatible `Read()` operations:

```cpp
archive.Read("sampleRate", sampleRate);
archive.Read("enabled", enabled);
```

The core library deliberately does not require Archives to derive from a common runtime-polymorphic base class.

This allows Archive operations to be resolved at compile time and enables highly specialized Archives without forcing virtual dispatch into every property operation.

### `KeyValueArchive`

`KeyValueArchive` is supplied as a small reference implementation and is used by the examples and host tests.

It stores primitive values as name/value pairs.

It is **not** intended to become the canonical persistence representation for ESPressio Serializable.

Its purpose is to:

* demonstrate the Archive contract;
* provide a simple mechanism for examples;
* allow the core serialization traversal to be tested without a third-party dependency; and
* provide a convenient starting point when implementing new Archives.

## Basic Serialization Example

```cpp
#include <Arduino.h>
#include <ESPressio_Serializable.hpp>

using namespace ESPressio;

class DeviceConfiguration final
    : public Serializable::Serializable<DeviceConfiguration> {

    ESPRESSIO_SERIALIZABLE_TYPE(DeviceConfiguration)

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

void setup() {
    Serial.begin(115200);

    DeviceConfiguration configuration;
    Serializable::KeyValueArchive archive;

    configuration.Serialize(archive);

    for (const auto& entry : archive.GetEntries()) {
        Serial.printf(
            "%s = %s\n",
            entry.Name.c_str(),
            entry.Value.c_str()
        );
    }
}

void loop() {
}
```

The important point is that `DeviceConfiguration` contains no `KeyValueArchive`-specific functionality.

Replacing `KeyValueArchive` with a future `JsonArchive`, `NVSArchive`, or another compatible Archive does not require changing the serialization declaration in `DeviceConfiguration`.

## Deserialization Example

The same property declarations are used in the opposite direction:

```cpp
Serializable::KeyValueArchive persisted;

persisted.Write("minimumSpeed", 10);
persisted.Write("maximumSpeed", 240);
persisted.Write("reversed", true);

MotorConfiguration configuration;

configuration.Deserialize(persisted);
```

Each declared property is looked up by its serialized name and, when available, assigned back to the corresponding member.

## Implementing a Custom Archive

The Archive contract is intentionally small.

For example, a diagnostic output Archive can be implemented as:

```cpp
class DebugArchive {
    public:
        template<typename TValue>
        void Write(
            const char* name,
            const TValue& value
        ) {
            Serial.print(name);
            Serial.print(" = ");
            Serial.println(value);
        }
};
```

It can immediately be used with any compatible Serializable object:

```cpp
Example example;
DebugArchive archive;

example.Serialize(archive);
```

No changes to `Example` or the ESPressio Serializable core are necessary.


## Transparent Nested Objects and Collections

Nested traversal is implemented by the **common Serializable core**, rather than separately by JSON, CBOR, Binary, or other Archive implementations.

The core converts the declared object graph into a representation-neutral `SerializationNode` tree.

This means that a property can itself be another Serializable object:

```cpp
class Position
    : public Serializable::Serializable<Position> {
    // ...
};

class Robot
    : public Serializable::Serializable<Robot> {

    ESPRESSIO_SERIALIZABLE_TYPE(Robot)

    private:
        Position _position;

    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("position", _position)
        )
};
```

The same declaration works with every tree-aware Archive:

```cpp
Serializable::BinaryArchive binary;
robot.Serialize(binary);

Serializable::CborArchive cbor;
robot.Serialize(cbor);

Serializable::JsonArchive json;
robot.Serialize(json);
```

No format-specific code is required in `Robot` or `Position`.

### Supported Common Property Types

The shared traversal layer currently handles:

* nested `Serializable<T>` objects;
* `std::array<T, N>`;
* `std::vector<T>`;
* collections containing nested Serializable objects;
* arbitrarily nested arrays/vectors of supported values;
* enumerations through their underlying integer type;
* signed integers;
* unsigned integers;
* `bool`;
* `float`;
* `double`;
* `std::string`.

For example:

```cpp
class Device
    : public Serializable::Serializable<Device> {

    ESPRESSIO_SERIALIZABLE_TYPE(Device)

    private:
        Position _position;
        std::array<uint16_t, 3> _calibration;
        std::vector<Sensor> _sensors;

    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("position", _position),
            ESPRESSIO_PROPERTY("calibration", _calibration),
            ESPRESSIO_PROPERTY("sensors", _sensors)
        )
};
```

The serializer-specific layer only needs to understand the common node types:

```text
Object
Array
Null
Boolean
SignedInteger
UnsignedInteger
Float32
Float64
String
```

Consequently, support for a new nested/container type is normally added once in the common traversal layer and immediately becomes available to every compatible serializer.

### Representation-Neutral Object Tree

The internal relationship is now:

```text
Serializable<T>
      |
      v
Property traversal
      |
      v
SerializationNode tree
      |
      +--------+--------+--------+
      |        |        |        |
      v        v        v        v
    JSON      CBOR    Binary    Future
```

This avoids implementing recursive object traversal separately for every representation and significantly reduces the opportunity for behavioural differences between serializers.


## Collections

Support for common collection types is also planned.

Expected supported types include:

* `std::array<T, N>`
* `std::vector<T>`
* collections containing Serializable types

Future consideration may also be given to:

* `std::optional<T>`
* `std::map<K, V>`
* `std::unordered_map<K, V>`

Collection support should remain Archive-independent wherever possible.

## Schema Evolution

Persisted embedded configuration often outlives the firmware version which originally created it.

For this reason, future versions of ESPressio Serializable are intended to support property metadata such as:

* aliases for renamed properties;
* required/optional properties;
* default values;
* read-only properties;
* sensitive properties;
* schema versions; and
* migration mechanisms.

For example, a future API may permit a property currently called:

```text
sampleRate
```

to continue accepting an older persisted name:

```text
samplingFrequency
```

without requiring the implementing class to maintain duplicate members.

## Examples

The repository currently contains:

### `BasicSerialization`

Demonstrates declaring primitive serializable members and serializing them into the reference `KeyValueArchive`.

### `DeserializeConfiguration`

Demonstrates restoring persisted values into an object through the same property declarations.

### `CustomArchive`

Demonstrates the intended Archive extension point by implementing a simple output Archive without modifying the Serializable core.


### `ESPNowSerializableMesh`

Demonstrates using the same sketch on two or more ESP32 devices to broadcast a Serializable object over ESP-NOW. The object is streamed directly into CBOR, fragmented into ESP-NOW-compatible packets, identified by its originating ESP32 MAC address, reassembled outside the Wi-Fi callback, and deserialized back into the original type.

## Host Tests

The core library can be compiled and tested on a host machine without requiring an ESP32.

```sh
cmake -S tests -B build/tests
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure
```

This is intentional: the serialization metadata and traversal machinery should remain independently testable from Arduino and ESP32-specific functionality.


## v0.4 Core Capabilities

Version `0.4.0` extends the common traversal/schema layer rather than adding format-specific special cases.

### Arduino `String`

The portable core does not include `Arduino.h`. Projects that need Arduino `String` support explicitly opt in with:

```cpp
#include <ESPressio_Serializable_Arduino.hpp>
```

This header specializes the portable `SerializationAdapter<T>` customization point for Arduino `String`.

### Property Metadata

Properties can be decorated fluently:

```cpp
ESPRESSIO_PROPERTY("name", _name)
    .Required()
    .Alias("oldName")
    .Sensitive()
```

Convenience macros are also available:

```cpp
ESPRESSIO_PROPERTY_REQUIRED("id", _id)
ESPRESSIO_PROPERTY_READONLY("serialNumber", _serialNumber)
ESPRESSIO_PROPERTY_SENSITIVE("password", _password)
```

Current metadata flags are `Required`, `ReadOnly`, and `Sensitive`.

### Aliases and Backwards Compatibility

A property can accept up to four historic names:

```cpp
ESPRESSIO_PROPERTY("sampleRate", _sampleRate)
    .Alias("samplingFrequency")
    .Alias("frequency")
```

Serialization always uses the canonical current name. Deserialization attempts the canonical name first and then its aliases.

### Versioned Schemas and Migrations

Types default to schema version `1`.

```cpp
ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(3)
```

Every Serializable object receives a `__schemaVersion` property.

A type can optionally implement:

```cpp
static bool Migrate(
    ESPressio::Serializable::SerializationNode& node,
    uint32_t fromVersion,
    uint32_t toVersion
);
```

Migrations run one version at a time before property deserialization and operate on the common representation-neutral tree.

### `std::optional<T>`

`std::optional<T>` is supported. An empty optional is represented as `Null`; a populated optional is represented as its contained value.

### Map-like Containers

The common traversal supports `std::map<TKey, TValue>` and `std::unordered_map<TKey, TValue>`. Maps are represented as arrays of `{ "key": ..., "value": ... }` entries so non-string key types remain portable across serializers.

### ESP32 Preferences / NVS

ESP32 Preferences support is opt-in:

```cpp
#include <ESPressio_Serializable_NVS.hpp>
```

`NvsArchive` stores the common Binary Archive payload as a byte blob in a Preferences/NVS entry.

### Compile-time Diagnostics

Unsupported property types now produce targeted compile-time errors. Raw pointers specifically recommend `std::optional<T>` or a custom `SerializationAdapter<T>`, while unsupported class types recommend `Serializable<T>` or an adapter specialization.



## v0.5 Core Capabilities

Version `0.6.0` adds property defaults and validators, migration helpers, set/list/deque traversal, enum string mappings, sensitive-property policies, strict numeric conversion checks, and a direct JSON streaming serializer for large object graphs.

### Defaults and Validation

```cpp
ESPRESSIO_PROPERTY("port", _port)
    .Default(80)
    .Range(1, 65535)
    .Validate(&ValidatePort);
```

A missing non-required property receives its declared default. Values read from canonical names or aliases are validated before deserialization succeeds. Numeric ranges are additionally checked before narrowing integer conversions.

### Migration Helpers

`ESPressio::Serializable::Migration` provides `Rename`, `Remove`, `Move`, and `ResolveObjectPath` helpers for common schema transformations.

### Additional Containers

The common traversal now supports `std::set`, `std::unordered_set`, `std::list`, and `std::deque` in addition to the existing vector/array/map types.

### Enum String Mappings

```cpp
ESPRESSIO_ENUM_MAPPING(Mode,
    ESPRESSIO_ENUM_VALUE(Mode::Off, "off"),
    ESPRESSIO_ENUM_VALUE(Mode::On,  "on")
)
```

Mapped enums serialize by name while still accepting legacy numeric enum payloads during deserialization.

### Sensitive-property Policies

Tree-aware archives expose `Policy()` and support `Include`, `Redact`, or `Omit` for properties marked `.Sensitive()`.

### Large Object Streaming

`ESPressio_Serializable_JSONStream.hpp` provides `JsonStreamSerializer`, which writes directly to Arduino `Print` without first constructing a `SerializationNode` tree. This substantially lowers peak memory for large forward-only JSON serialization jobs.


## v0.7 Hardening, Compatibility and Tooling

Version `0.7.0` adds a reproducible hardening layer around the serialization framework: deterministic malformed-input mutation tests, libFuzzer entry points for Binary and CBOR, immutable wire-format compatibility policy/test vectors, nested diagnostic paths with configurable `FailFast`/`CollectAll` behavior, ESP32 internal-RAM/PSRAM allocator adapters, a portable static-pool allocator, additional `SchemaInspector<T>` JSON/CSV/Mermaid outputs, migration regression tests, benchmark firmware, and further transport examples.

Hardware benchmark results are intentionally **not fabricated or generalized from host measurements**. The repository supplies an ESP32 benchmark harness and documentation so numbers can be collected on explicitly identified physical targets and committed reproducibly.

The direct streaming serializers remain the preferred path for very large forward-only output. JSON streaming deserialization continues to use ArduinoJson's document model; callers can use the filter overload to limit materialized fields, while CBOR streaming can avoid retaining the complete encoded payload.

## Development Roadmap

ESPressio Serializable `0.9.0` is considered **feature-complete for the planned `1.0.0` scope**.

The remaining work before `1.0.0` is release hardening and downstream validation rather than additional feature development:

1. compile all examples against representative ESP32, ESP32-S3, and ESP32-C3 PlatformIO targets;
2. run JSON, CBOR, native Binary, NVS, Stream, TCP, and ESP-NOW examples on physical hardware;
3. verify clean installation and example compilation in the Arduino IDE;
4. run the benchmark harness on identified boards and record real RAM, flash, and runtime measurements;
5. run the fuzz harnesses for extended periods and preserve useful corpora/regressions;
6. verify frozen Binary/CBOR compatibility vectors and cross-version schema migration fixtures;
7. review the complete public API, macro names, serializer names, result/error types, and wire-format contracts for final `1.0.0` stability;
8. add/expand CI for host tests and representative PlatformIO compile targets;
9. validate the library from downstream consuming projects with no local-path assumptions;
10. fix only defects, inconsistencies, portability issues, or release blockers discovered during the `0.9.x` validation cycle.

### Feature-complete `1.0.0` scope

The following capabilities are already implemented and are **not roadmap items**:

* declarative Serializable property metadata;
* defaults, validation callbacks, numeric range validation, aliases, and read-only/sensitive metadata;
* schema versions and migration support;
* rename/move/remove/indexed migration helpers;
* nested Serializable objects;
* `std::optional`, arrays, vectors, sets, unordered sets, lists, deques, maps, and unordered maps;
* enum name/string mappings;
* JSON, CBOR, native Binary, NVS, and Arduino Stream adapters;
* serializer-aware sensitive-property include/redact/omit policies;
* direct streaming JSON, CBOR, and Binary serialization;
* streaming JSON and CBOR deserialization;
* structured validation/deserialization diagnostics with fail-fast or collect-all behavior;
* configurable allocator strategies, including static-pool and ESP32/PSRAM-oriented allocators;
* schema introspection and Markdown/JSON/CSV/Mermaid documentation generation;
* optional property-name elimination for constrained binary-only builds;
* malformed-input mutation tests and fuzz harnesses;
* frozen wire-format compatibility vectors;
* schema-migration regression tests;
* transport examples including ESP-NOW, Serial, and TCP;
* CRC32-framed stream transport support; and
* reproducible ESP32 benchmark harnesses.

## Design Principle

An implementing type should only need to answer one question:

> **Which members constitute my serializable state?**

It should not need to know whether those members will ultimately become JSON, CBOR, NVS entries, binary data, diagnostic output, or some future representation which did not exist when the type was written.

That separation is the fundamental purpose of ESPressio Serializable.

## v0.6 Advanced Streaming, Diagnostics and Schema Tooling

Version `0.6.0` adds optional stream-oriented JSON/CBOR/native Binary I/O, structured `DeserializationResult` diagnostics, indexed migration paths such as `items[3].name`, allocator customization via `ESPRESSIO_SERIALIZATION_ALLOCATOR`, `SchemaInspector<T>` documentation generation, and a nameless native-binary mode for extremely constrained builds.

### Streaming input/output

`JsonStreamDeserializer` passes the Arduino `Stream` directly to ArduinoJson and supports ArduinoJson filtering. `CborStreamDeserializer` consumes a stream without requiring the caller to first materialize the payload. `CborStreamSerializer`, `BinaryStreamSerializer<>`, and `NamelessBinaryStreamSerializer` write directly to Arduino `Print`.

### Structured errors

Use `object.DeserializeDetailed(archive)` to receive `DeserializationResult`, containing one or more `SerializationIssue` records with an error code, property path, and message. Existing `Deserialize()` remains as a boolean compatibility wrapper.

### Indexed migration paths

`Migration::ResolvePath()` understands paths such as `devices[2].settings.name`; `RemoveAt()` and `MovePath()` operate on the same syntax.

### Allocator customization

Define `ESPRESSIO_SERIALIZATION_ALLOCATOR` before including ESPressio headers to replace the allocator used by node-tree child containers and serialization buffers.

### Schema documentation

`SchemaInspector<MyType>::Properties()` exposes property metadata programmatically and `SchemaInspector<MyType>::Markdown()` produces a Markdown schema table.

### Nameless binary-only builds

`ESPRESSIO_PROPERTY_NAMELESS(member)` removes the textual property name from that descriptor. Pair this with `NamelessBinaryStreamSerializer` when the producer/consumer share the same schema and property order. This mode intentionally trades self-description and cross-format compatibility for minimum binary size.
