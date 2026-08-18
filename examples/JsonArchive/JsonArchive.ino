#include <Arduino.h>
#include <ESPressio_Serializable_JSON.hpp>

using namespace ESPressio;

class Settings final
    : public Serializable::Serializable<Settings> {

    ESPRESSIO_SERIALIZABLE_TYPE(Settings)

    private:
        uint32_t _sampleRate = 1000;
        float _threshold = 0.5f;
        bool _enabled = true;

    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("sampleRate", _sampleRate),
            ESPRESSIO_PROPERTY("threshold", _threshold),
            ESPRESSIO_PROPERTY("enabled", _enabled)
        )
};

void setup() {
    Serial.begin(115200);

    Settings settings;
    Serializable::JsonArchive archive;

    settings.Serialize(archive);

    // ArduinoJson can serialize directly to Arduino Stream.
    archive.SavePretty(Serial);
    Serial.println();
}

void loop() {
}
