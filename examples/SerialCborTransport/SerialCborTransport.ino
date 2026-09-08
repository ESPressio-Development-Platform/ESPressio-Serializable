#include <Arduino.h>
#include <ESPressio_Serializable_CBORStream.hpp>
using namespace ESPressio;
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<Packet>) [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: sizeof(Serializable::Serializable<Packet>) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class Packet:public Serializable::Serializable<Packet>{ESPRESSIO_SERIALIZABLE_TYPE(Packet) private:uint32_t _sequence=1; public:ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("sequence",_sequence))};
void setup(){Serial.begin(115200);delay(500);Packet p;Serializable::CborStreamSerializer out(Serial);out.Serialize(p);}
void loop(){}
