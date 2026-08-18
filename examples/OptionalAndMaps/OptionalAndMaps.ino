#include <Arduino.h>
#include <map>
#include <optional>
#include <ESPressio_Serializable_Arduino.hpp>
#include <ESPressio_Serializable_CBOR.hpp>

using namespace ESPressio;

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
