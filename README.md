# ESPressio Serializable

Declarative serialization components for the Flowduino ESPressio
Development Platform.

## Latest Stable Version

**0.9.0**

## ESPressio Development Platform

ESPressio is a collection of discrete, composable component libraries
built around a common development ethos:

-   **Light-weight** --- minimise memory consumption and runtime
    overhead without sacrificing correctness.
-   **Ease of use** --- provide strongly typed, developer-friendly
    abstractions over lower-level facilities.
-   **Object-oriented** --- a type for everything, and everything in a
    type.
-   **SOLID** --- favour focused responsibilities, extensibility,
    substitutable abstractions, narrow interfaces, and dependency
    inversion wherever practical on embedded C++ platforms.

## License

Licensed under the **Apache License 2.0**. See [LICENSE](LICENSE).

## ESPressio Library Dependencies

ESPressio is designed as a modular ecosystem of independently useful
libraries, with required dependencies kept explicit and optional
integrations introduced only when the corresponding functionality is
selected.

For a complete overview of required dependencies, opt-in dependencies,
and the overall hierarchy, see:

**[ESPressio Library Dependency Chart](ESPRESSIO_DEPENDENCY_CHART.md)**

-   **Solid relationships** represent required ESPressio dependencies.
-   **Dashed relationships** represent opt-in dependencies introduced
    only by the corresponding feature, integration, type, or header.

### Required ESPressio dependencies

None.

## Declarative schema model

Serializable types declare their schema alongside the type itself:

``` cpp
class Example :
    public ESPressio::Serializable::
        SerializableBase<Example> {

public:
    uint32_t Id = 0;
    String Name;

    ESPRESSIO_SERIALIZABLE_TYPE(Example)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)

    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY("id", Id),
        ESPRESSIO_PROPERTY("name", Name)
    )
};
```

The same declaration can be consumed by multiple archive
representations.

## Representations

The framework supports JSON, CBOR and Binary representations, including
direct CBOR/Binary paths rather than requiring JSON as an intermediate
representation.

Streaming readers/writers are available where supported by the selected
archive.

## Value support

The framework supports common scalar and compound values including
arithmetic types, nested Serializable objects, arrays/containers,
Arduino `String`, `std::optional`, enum mapping, and supported map/set
forms.

## Validation

Properties and objects can participate in validation, including numeric
constraints and application-defined validation callbacks.

## Schema evolution

Serializable types declare schema versions. Migration helpers, aliases
and defaults allow older persisted or transported representations to
evolve deliberately.

## Redaction

Schema metadata can identify fields that should be redacted in
diagnostic/log-oriented representations.

## Compile-time diagnostics

Where an invalid or unsupported schema declaration can be detected
statically, the library is designed to fail at compile time rather than
defer the problem to runtime.

## ESPressio integrations

Serializable remains independent. Other libraries opt into it:

``` text
Units
    -> optional Serializable Unit variants

Event
    -> optional Serializable Events
    -> Event Transport payloads
```

Ordinary usage of those libraries remains serialization-free.

## Design goals

-   One declarative schema per type.
-   Representation-neutral metadata.
-   Embedded-friendly archives.
-   Direct CBOR/Binary support.
-   Validation and schema evolution.
-   Compile-time diagnostics where possible.
-   Optional rather than ecosystem-wide dependency.
