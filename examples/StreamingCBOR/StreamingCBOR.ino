#include <Arduino.h>
#include <ESPressio_Serializable_CBORStream.hpp>
using namespace ESPressio;
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
