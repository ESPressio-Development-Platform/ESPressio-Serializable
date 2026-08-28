# Getting Started

Include the core umbrella:

```cpp
#include <ESPressio_Serializable.hpp>
```

Declare a model once:

```cpp
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

The declaration describes the model's serializable state, not its representation or storage destination.

The same object can then be sent through JSON, CBOR, ESPB Binary, direct Binary, streaming/custom archives, or optional protected serialization.

## Next steps

- [Declaring Serializable Types](Declaring-Serializable-Types)
- [Validation and Diagnostics](Validation-and-Diagnostics)
- [Schema Evolution](Schema-Evolution)
- [Memory and Decode Limits](Memory-and-Decode-Limits)