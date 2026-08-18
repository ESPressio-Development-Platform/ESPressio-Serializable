#include <Arduino.h>
#include <ESPressio_Serializable_BinaryStream.hpp>
using namespace ESPressio;
class TinyState : public Serializable::Serializable<TinyState> {
    ESPRESSIO_SERIALIZABLE_TYPE(TinyState)
    uint16_t _counter = 0;
    bool _enabled = true;
public:
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY_NAMELESS(_counter),
        ESPRESSIO_PROPERTY_NAMELESS(_enabled)
    )
};
void setup(){
    Serial.begin(115200);
    TinyState state;
    Serializable::NamelessBinaryStreamSerializer writer(Serial);
    writer.Serialize(state);
}
void loop(){}
