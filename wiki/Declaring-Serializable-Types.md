# Declaring Serializable Types

A Serializable model declares one authoritative schema for its persisted/interchanged state.

Typical structure:

```cpp
class Example final
    : public ESPressio::Serializable::Serializable<Example> {

    ESPRESSIO_SERIALIZABLE_TYPE(Example)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)

private:
    uint32_t _value = 0;

public:
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY("value", _value)
    )
};
```

## Model responsibility

The model owns its state and schema metadata. It should not contain parallel `ToJson()`, `ToCbor()`, or persistence-specific methods for the same state.

## Archive responsibility

Archives own representation. They consume the declared properties and emit/restore the selected representation.

## Schema version

Assign a schema version whenever data may persist across firmware versions. Schema evolution facilities can then migrate older representations into the current model.

## Stable names

Property names become part of serialized contracts. Treat renaming as schema evolution rather than a cosmetic refactor.