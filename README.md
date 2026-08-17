# ESPressio Serializable

Compile-time declarative Serialization components of the Flowduino ESPressio Development Platform.

This starter implementation is intentionally small and representation-neutral. It provides the base machinery for declaring which members of a C++ type are serializable without coupling the type to JSON, CBOR, NVS, a filesystem, or any other persistence/transport format.

## Design objectives

* Light-weight.
* Developer-friendly.
* Object-oriented.
* SOLID-oriented.
* No ESPressio intra-library dependencies.
* No serialization-format dependencies.
* Compile-time property metadata.
* No RTTI requirement for the core implementation.
* No heap allocation performed by property descriptors.
* Extensible through Archive implementations.

## Namespace

All library types are in:

```cpp
namespace ESPressio::Serializable
```

## Basic usage

```cpp
#include <ESPressio_Serializable.hpp>

using namespace ESPressio;

class DeviceConfiguration final
    : public Serializable::Serializable<DeviceConfiguration> {

    ESPRESSIO_SERIALIZABLE_TYPE(DeviceConfiguration)

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
```

The macros are deliberately limited to schema declaration. They generate no runtime registry.

## Archive model

`Serializable<T>::Serialize()` accepts any archive exposing:

```cpp
archive.Write(const char* name, const TValue& value);
```

`Deserialize()` accepts any archive exposing:

```cpp
archive.Read(const char* name, TValue& value);
```

This means JSON support can live in a separate adapter and depend on ArduinoJson without introducing that dependency into the core library.

## Included reference archive

`KeyValueArchive` is a deliberately simple reference/example implementation. It is useful for tests and demonstrating the Archive contract; it is not intended to become the library's canonical persistence format.

## Examples

* `BasicSerialization` - serializes primitive members into the reference archive.
* `DeserializeConfiguration` - restores values into an object.
* `CustomArchive` - demonstrates how trivial a new output adapter can be.

## Host tests

The starter repository follows the host-test pattern used by newer ESPressio components.

```sh
cmake -S tests -B build/tests
cmake --build build/tests
ctest --test-dir build/tests --output-on-failure
```

## Suggested next milestones

1. Nested `Serializable<T>` member support.
2. `std::array` and `std::vector`.
3. Arduino `String` support in format adapters.
4. Property flags (`Required`, `ReadOnly`, `Sensitive`).
5. Property aliases for backwards-compatible schema evolution.
6. Versioned schemas/migrations.
7. JSON Archive adapter.
8. CBOR Archive adapter.
9. ESP32 Preferences/NVS Archive adapter.
10. Compile-time tests for unsupported member types.

## C++ standard

The current implementation requires C++17 for `std::apply`, fold expressions, `std::void_t`, and inline variable templates.

## License

Apache License 2.0.
