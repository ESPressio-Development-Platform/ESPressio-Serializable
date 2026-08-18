#include <Arduino.h>
#include <ESPressio_Serializable.hpp>
using namespace ESPressio;
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
