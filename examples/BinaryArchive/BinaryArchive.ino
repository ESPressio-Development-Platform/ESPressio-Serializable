#include <Arduino.h>
#include <ESPressio_Serializable_Binary.hpp>

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
    Serializable::BinaryArchive archive;

    settings.Serialize(archive);

    const auto bytes = archive.GetData();

    Serial.printf(
        "Binary payload: %u bytes\n",
        static_cast<unsigned>(bytes.size())
    );
}

void loop() {
}
