#include <Arduino.h>
#include <ESPressio_Serializable_Binary.hpp>
#include <ESPressio_Serializable_Stream.hpp>

using namespace ESPressio;

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<Packet>) [0 bytes dynamic allocation]
 * Members:
 * - _active (bool): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: sizeof(Serializable::Serializable<Packet>) + 1 bytes known members [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class Packet final
    : public Serializable::Serializable<Packet> {

    ESPRESSIO_SERIALIZABLE_TYPE(Packet)

    private:
        uint32_t _sequence = 42;
        bool _active = true;

    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("sequence", _sequence),
            ESPRESSIO_PROPERTY("active", _active)
        )
};

void setup() {
    Serial.begin(115200);

    Packet packet;

    // For demonstration this writes a length-prefixed native binary payload
    // to Serial. The same API can be used with HardwareSerial, WiFiClient,
    // File, etc. because they derive from Arduino Stream.
    Serializable::StreamArchive::Serialize<
        Serializable::BinaryArchive
    >(
        packet,
        Serial
    );
}

void loop() {
}
