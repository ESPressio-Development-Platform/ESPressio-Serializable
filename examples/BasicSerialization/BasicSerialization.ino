#include <Arduino.h>
#include <ESPressio_Serializable.hpp>

using namespace ESPressio;

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<DeviceConfiguration>) [0 bytes dynamic allocation]
 * Members:
 * - _threshold (float): 4 bytes [0 bytes dynamic allocation]
 * - _loggingEnabled (bool): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: sizeof(Serializable::Serializable<DeviceConfiguration>) + 5 bytes known members [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
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
