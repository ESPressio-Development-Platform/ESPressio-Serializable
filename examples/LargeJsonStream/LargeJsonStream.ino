#include <Arduino.h>
#include <vector>
#include <ESPressio_Serializable_JSONStream.hpp>

using namespace ESPressio;

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<Telemetry>) [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: sizeof(Serializable::Serializable<Telemetry>) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
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
