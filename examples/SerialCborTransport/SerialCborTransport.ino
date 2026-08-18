#include <Arduino.h>
#include <ESPressio_Serializable_CBORStream.hpp>
using namespace ESPressio;
class Packet:public Serializable::Serializable<Packet>{ESPRESSIO_SERIALIZABLE_TYPE(Packet) private:uint32_t _sequence=1; public:ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("sequence",_sequence))};
void setup(){Serial.begin(115200);delay(500);Packet p;Serializable::CborStreamSerializer out(Serial);out.Serialize(p);}
void loop(){}
