#include <Arduino.h>
#include <ESPressio_Serializable_Arduino.hpp>
#include <ESPressio_Serializable_NVS.hpp>

using namespace ESPressio;

class Settings final
    : public Serializable::Serializable<Settings> {
    ESPRESSIO_SERIALIZABLE_TYPE(Settings)
    private:
        String _deviceName = "ESP32";
        uint32_t _sampleRate = 1000;
    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("deviceName", _deviceName),
            ESPRESSIO_PROPERTY("sampleRate", _sampleRate)
        )
};

void setup() {
    Serial.begin(115200);

    Settings settings;
    Serializable::NvsArchive output;
    settings.Serialize(output);
    output.Save("espressio", "settings");

    Serializable::NvsArchive input;
    if (input.Load("espressio", "settings")) {
        Settings restored;
        restored.Deserialize(input);
    }
}

void loop() {}
