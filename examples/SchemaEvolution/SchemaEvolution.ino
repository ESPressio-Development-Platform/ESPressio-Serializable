#include <Arduino.h>
#include <ESPressio_Serializable_Arduino.hpp>
#include <ESPressio_Serializable_JSON.hpp>

using namespace ESPressio;

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<Settings>) [0 bytes dynamic allocation]
 * Members:
 * - _deviceName (String): 12 bytes [Capacity + 1 bytes (Arduino String backing buffer when allocated)]
 * Total Memory: sizeof(Serializable::Serializable<Settings>) + 12 bytes known members [_deviceName: Capacity + 1 bytes (Arduino String backing buffer when allocated)]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class Settings final
    : public Serializable::Serializable<Settings> {
    ESPRESSIO_SERIALIZABLE_TYPE(Settings)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(2)

    private:
        uint16_t _port = 80;
        String _deviceName = "ESP32";

    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("port", _port).Alias("httpPort"),
            ESPRESSIO_PROPERTY_REQUIRED("deviceName", _deviceName).Alias("name")
        )

        static bool Migrate(
            Serializable::SerializationNode&,
            uint32_t fromVersion,
            uint32_t toVersion
        ) {
            return fromVersion == 1 && toVersion == 2;
        }
};

void setup() { Serial.begin(115200); }
void loop() {}
