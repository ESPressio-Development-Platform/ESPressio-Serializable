#include <Arduino.h>
#include <ESPressio_Serializable_BinaryStream.hpp>
using namespace ESPressio;
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<TinyState>) [0 bytes dynamic allocation]
 * Members:
 * - _enabled (bool): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: sizeof(Serializable::Serializable<TinyState>) + 1 bytes known members [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
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
