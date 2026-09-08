#include <Arduino.h>
#include <ESPressio_Serializable.hpp>
using namespace ESPressio;
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<Settings>) [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: sizeof(Serializable::Serializable<Settings>) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class Settings : public Serializable::Serializable<Settings> {
    ESPRESSIO_SERIALIZABLE_TYPE(Settings)
    int _port = 80;
public:
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY_REQUIRED("port", _port).Range(1,65535)
    )
};
void setup(){
    Serial.begin(115200);
    auto markdown = Serializable::SchemaInspector<Settings>::Markdown();
    Serial.print(markdown.c_str());
}
void loop(){}
