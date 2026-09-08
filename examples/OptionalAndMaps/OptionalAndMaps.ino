#include <Arduino.h>
#include <map>
#include <optional>
#include <ESPressio_Serializable_Arduino.hpp>
#include <ESPressio_Serializable_CBOR.hpp>

using namespace ESPressio;

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<Device>) [0 bytes dynamic allocation]
 * Members:
 * - _namedReadings (std::map<String, float>): 24 bytes [N * (16 bytes red-black-tree node overhead + 16 bytes value)]
 * Total Memory: sizeof(Serializable::Serializable<Device>) + 24 bytes known members [_namedReadings: N * (16 bytes red-black-tree node overhead + 16 bytes value)]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class Device final
    : public Serializable::Serializable<Device> {
    ESPRESSIO_SERIALIZABLE_TYPE(Device)
    private:
        std::optional<int32_t> _lastReading;
        std::map<String, float> _namedReadings;
    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("lastReading", _lastReading),
            ESPRESSIO_PROPERTY("namedReadings", _namedReadings)
        )
};

void setup() { Serial.begin(115200); }
void loop() {}
