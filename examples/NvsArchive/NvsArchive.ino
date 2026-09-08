#include <Arduino.h>
#include <ESPressio_Serializable_Arduino.hpp>
#include <ESPressio_Serializable_NVS.hpp>

using namespace ESPressio;

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<Settings>) [0 bytes dynamic allocation]
 * Members:
 * - _sampleRate (uint32_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: sizeof(Serializable::Serializable<Settings>) + 4 bytes known members [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
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
