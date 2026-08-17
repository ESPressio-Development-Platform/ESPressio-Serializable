# ESPressio Serializable

Serialization and Deserialization Components of the Flowduino ESPressio Development Platform

Provides a foundation for declaratively describing the serializable state of C++ types and serializing or deserializing that state through interchangeable Archive implementations.

The core library deliberately does not prescribe JSON, CBOR, NVS, filesystem, network, or other storage/transport representations. Instead, implementing classes expose their serializable members once, while Archive implementations determine how those members are represented.

## Latest Stable Version

ESPressio Serializable is currently under initial development.

The starter implementation is version `0.1.0` and should be considered a development version rather than a stable public release.

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

    ESPressio::Serializable::Serializable<T>
    ESPressio::Serializable::SerializableBase<T>
    ESPressio::Serializable::SerializationProperty<TObject, TValue>
    ESPressio::Serializable::KeyValueArchive

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

    lib_deps =
        flowduino/ESPressio-Serializable@^0.1.0

Alternatively, once the GitHub repository is public, the latest development sources can be referenced directly:

    lib_deps =
        https://github.com/Flowduino/ESPressio-Serializable.git

Please note that using the GitHub repository directly will use the latest commits pushed into the repository, so volatility is possible.

## What is ESPressio Serializable?

ESPressio Serializable provides a common mechanism by which a C++ type can describe the members that constitute its serializable state.

C++17 does not provide the runtime reflection or user-defined annotation facilities commonly available in languages such as C# or Java. ESPressio Serializable therefore provides a small compile-time metadata mechanism which allows an implementing class to declaratively identify its serializable members.

For example:

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

The implementing class describes **what** is serializable, but does not describe **how** that state is represented.

This distinction is fundamental to the design of the library.

A JSON Archive may serialize the object as JSON. An NVS Archive may persist the same properties into ESP32 Preferences. A CBOR Archive may encode the same object into a compact binary representation.

The implementing class remains unchanged.

## Why not simply implement `ToJson()` and `FromJson()`?

Doing so couples the implementing type directly to JSON.

If that same object later needs to be persisted to NVS, transferred as CBOR, written to a binary file, or represented in some other format, the object itself must either acquire additional responsibilities or its JSON-specific implementation must be worked around.

ESPressio Serializable instead applies the Dependency Inversion Principle:

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

The object owns its state.

The property declarations describe that state.

The Archive determines its representation.

## Understanding the Components of ESPressio Serializable

Before looking at complete examples, it is useful to understand the individual components and their responsibilities.

### `Serializable<T>`

`Serializable<T>` is the normal base type from which a serializable class inherits.

It is currently an alias of `SerializableBase<T>`:

    template<typename TDerived>
    using Serializable = SerializableBase<TDerived>;

The library uses the Curiously Recurring Template Pattern (CRTP), meaning that the implementing type passes itself as the template parameter:

    class Settings
        : public ESPressio::Serializable::Serializable<Settings> {
    };

This allows serialization traversal to be resolved at compile time without requiring runtime type discovery.

### `SerializableBase<T>`

`SerializableBase<T>` implements the common `Serialize()` and `Deserialize()` traversal.

Calling:

    configuration.Serialize(archive);

enumerates the properties declared by the implementing type and invokes:

    archive.Write(propertyName, propertyValue);

for each property.

Likewise:

    configuration.Deserialize(archive);

invokes:

    archive.Read(propertyName, propertyValue);

for each property.

The Archive interface is intentionally duck-typed. An Archive does not need to inherit from a specific base class in the initial implementation; it simply needs to provide compatible `Read()` and/or `Write()` operations.

### `SerializationProperty<TObject, TValue>`

A `SerializationProperty` associates a serialized property name with a pointer-to-member.

For example:

    ESPRESSIO_PROPERTY("sampleRate", _sampleRate)

ultimately describes the relationship between:

    Serialized Name        C++ Member
    ---------------        ----------
    sampleRate          -> _sampleRate

The descriptor itself does not store a copy of the member value.

Instead, it stores compile-time metadata through which the value can be obtained from a specific object instance.

### Serializable Declaration Macros

Three macros currently provide the annotation-like declaration syntax.

First, identify the implementing type:

    ESPRESSIO_SERIALIZABLE_TYPE(DeviceConfiguration)

Then declare its serializable properties:

    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY("sampleRate", _sampleRate),
        ESPRESSIO_PROPERTY("threshold", _threshold),
        ESPRESSIO_PROPERTY("loggingEnabled", _loggingEnabled)
    )

The macros are deliberately restricted to schema declaration.

They do not create a runtime property registry and they do not determine the output representation.

### `IsSerializable<T>`

`IsSerializable<T>` is a compile-time trait which determines whether a type exposes the Serializable property contract.

For example:

    static_assert(
        ESPressio::Serializable::IsSerializable<DeviceConfiguration>
    );

This will become increasingly useful as Archive implementations gain recursive support for nested serializable objects and collections.

### Archives

An Archive is responsible for converting between C++ values and some external representation.

For serialization, an Archive exposes compatible `Write()` operations:

    archive.Write("sampleRate", sampleRate);
    archive.Write("enabled", enabled);

For deserialization, it exposes compatible `Read()` operations:

    archive.Read("sampleRate", sampleRate);
    archive.Read("enabled", enabled);

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

The important point is that `DeviceConfiguration` contains no `KeyValueArchive`-specific functionality.

Replacing `KeyValueArchive` with a future `JsonArchive`, `NVSArchive`, or another compatible Archive does not require changing the serialization declaration in `DeviceConfiguration`.

## Deserialization Example

The same property declarations are used in the opposite direction:

    Serializable::KeyValueArchive persisted;

    persisted.Write("minimumSpeed", 10);
    persisted.Write("maximumSpeed", 240);
    persisted.Write("reversed", true);

    MotorConfiguration configuration;

    configuration.Deserialize(persisted);

Each declared property is looked up by its serialized name and, when available, assigned back to the corresponding member.

## Implementing a Custom Archive

The Archive contract is intentionally small.

For example, a diagnostic output Archive can be implemented as:

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

It can immediately be used with any compatible Serializable object:

    Example example;
    DebugArchive archive;

    example.Serialize(archive);

No changes to `Example` or the ESPressio Serializable core are necessary.

## Nested Serializable Types

Recursive serialization of nested Serializable objects is a planned core capability.

The intended usage is:

    class Position
        : public Serializable::Serializable<Position> {
        // x, y and z properties...
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

A representation-aware Archive will then be able to recursively process `_position` as another Serializable object.

This functionality is not yet implemented in the `0.1.0` starter.

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

    sampleRate

to continue accepting an older persisted name:

    samplingFrequency

without requiring the implementing class to maintain duplicate members.

## Examples

The repository currently contains:

### `BasicSerialization`

Demonstrates declaring primitive serializable members and serializing them into the reference `KeyValueArchive`.

### `DeserializeConfiguration`

Demonstrates restoring persisted values into an object through the same property declarations.

### `CustomArchive`

Demonstrates the intended Archive extension point by implementing a simple output Archive without modifying the Serializable core.

## Host Tests

The core library can be compiled and tested on a host machine without requiring an ESP32.

    cmake -S tests -B build/tests
    cmake --build build/tests
    ctest --test-dir build/tests --output-on-failure

This is intentional: the serialization metadata and traversal machinery should remain independently testable from Arduino and ESP32-specific functionality.

## Development Roadmap

The initial implementation deliberately establishes the smallest useful foundation before adding representation-specific functionality.

Likely next milestones are:

1. Nested Serializable object support.
2. `std::array` and `std::vector` support.
3. Arduino `String` support where appropriate.
4. Property metadata/options.
5. Property aliases and backwards-compatible schema evolution.
6. Versioned schemas and migration support.
7. JSON Archive adapter.
8. CBOR Archive adapter.
9. ESP32 Preferences/NVS Archive adapter.
10. Compile-time diagnostics for unsupported member types.

## Design Principle

An implementing type should only need to answer one question:

> **Which members constitute my serializable state?**

It should not need to know whether those members will ultimately become JSON, CBOR, NVS entries, binary data, diagnostic output, or some future representation which did not exist when the type was written.

That separation is the fundamental purpose of ESPressio Serializable.