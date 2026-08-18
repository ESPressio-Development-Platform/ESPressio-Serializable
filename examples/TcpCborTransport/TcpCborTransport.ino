#include <Arduino.h>
#include <WiFi.h>
#include <ESPressio_Serializable_CBORStream.hpp>
// Transport example skeleton: once a WiFiClient is connected, pass it directly
// to CborStreamSerializer/CborStreamDeserializer because WiFiClient is a Stream.
void setup(){Serial.begin(115200);}
void loop(){}
