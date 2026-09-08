#include <Arduino.h>
#include <vector>
#include <ESPressio_Serializable_JSONStream.hpp>

using namespace ESPressio;

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (standalone empty object occupies 1 byte; an eligible empty base may be optimized to 0 bytes).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class Telemetry final : public Serializable::Serializable<Telemetry> {
    ESPRESSIO_SERIALIZABLE_TYPE(Telemetry)
    private:
        std::vector<uint32_t> _samples;
    public:
        Telemetry() : _samples(1000, 42) {}
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("samples", _samples)
        )
};

void setup() {
    Serial.begin(115200);
    Telemetry telemetry;
    Serializable::JsonStreamSerializer serializer(Serial);
    serializer.Serialize(telemetry);
}

void loop() {}
