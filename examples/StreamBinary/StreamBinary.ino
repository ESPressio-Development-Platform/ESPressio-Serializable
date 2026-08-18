#include <Arduino.h>
#include <ESPressio_Serializable_Binary.hpp>
#include <ESPressio_Serializable_Stream.hpp>

using namespace ESPressio;

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
