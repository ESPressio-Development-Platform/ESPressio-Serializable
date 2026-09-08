#include <Arduino.h>
#include <ESPressio_Serializable_CBOR.hpp>

using namespace ESPressio;

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members:
 * - _threshold (float): 4 bytes [0 bytes dynamic allocation]
 * - _enabled (bool): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: 12 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
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
    Serializable::CborArchive archive;

    settings.Serialize(archive);

    const auto bytes = archive.GetData();

    Serial.printf(
        "CBOR payload: %u bytes\n",
        static_cast<unsigned>(bytes.size())
    );
}

void loop() {
}
