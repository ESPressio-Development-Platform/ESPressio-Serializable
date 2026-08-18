#include <Arduino.h>
#include <vector>
#include <ESPressio_Serializable_JSONStream.hpp>

using namespace ESPressio;

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
