#include <Arduino.h>
#include <ESPressio_Serializable.hpp>
using namespace ESPressio;
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (standalone empty object occupies 1 byte; an eligible empty base may be optimized to 0 bytes).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
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
