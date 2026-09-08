#include <Arduino.h>
#include <map>
#include <optional>
#include <ESPressio_Serializable_Arduino.hpp>
#include <ESPressio_Serializable_CBOR.hpp>

using namespace ESPressio;

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members:
 * - _namedReadings (std::map<String, float>): 24 bytes [N * (16 bytes red-black-tree node linkage + 16 bytes value); key/value: Capacity + 1 bytes backing buffer when allocated]
 * Total Memory: 28 bytes [_namedReadings: N * (16 bytes red-black-tree node linkage + 16 bytes value); _namedReadings: key/value: Capacity + 1 bytes backing buffer when allocated]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
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
