#include <Arduino.h>
#include <ESPressio_Serializable.hpp>

using namespace ESPressio;

class DeviceConfiguration final
    : public Serializable::Serializable<DeviceConfiguration> {

    ESPRESSIO_SERIALIZABLE_TYPE(DeviceConfiguration)

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

        uint32_t GetSampleRate() const {
            return _sampleRate;
        }
};

void setup() {
    Serial.begin(115200);

    DeviceConfiguration configuration;
    Serializable::KeyValueArchive archive;

    configuration.Serialize(archive);

    for (const auto& entry : archive.GetEntries()) {
        Serial.printf(
            "%s = %s\n",
            entry.Name.c_str(),
            entry.Value.c_str()
        );
    }
}

void loop() {
}
