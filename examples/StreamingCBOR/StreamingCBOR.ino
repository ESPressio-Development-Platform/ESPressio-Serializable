#include <Arduino.h>
#include <ESPressio_Serializable_CBORStream.hpp>
using namespace ESPressio;
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (standalone empty object occupies 1 byte; an eligible empty base may be optimized to 0 bytes).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class Packet : public Serializable::Serializable<Packet> {
    ESPRESSIO_SERIALIZABLE_TYPE(Packet)
    uint32_t _sequence = 1;
public:
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY("sequence", _sequence)
    )
};
void setup(){
    Serial.begin(115200);
    Packet packet;
    Serializable::CborStreamSerializer writer(Serial);
    writer.Serialize(packet);
    // On a receiving Stream:
    // Serializable::CborStreamDeserializer reader(stream);
    // auto result = reader.Deserialize(packet);
}
void loop(){}
