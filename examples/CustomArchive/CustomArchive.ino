#include <Arduino.h>
#include <ESPressio_Serializable.hpp>

using namespace ESPressio;

/*
 * Demonstrates the intended extension point.
 *
 * A real JSON archive could forward these calls to ArduinoJson without
 * requiring ESPressio Serializable itself to depend on ArduinoJson.
 */
class DebugArchive {
    public:
        template<typename TValue>
        void Write(const char* name, const TValue& value) {
            Serial.print(name);
            Serial.print(" = ");
            Serial.println(value);
        }
};

class Example final : public Serializable::Serializable<Example> {

    ESPRESSIO_SERIALIZABLE_TYPE(Example)

    private:
        int _number = 42;
        float _ratio = 3.5f;

    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("number", _number),
            ESPRESSIO_PROPERTY("ratio", _ratio)
        )
};

void setup() {
    Serial.begin(115200);

    Example example;
    DebugArchive archive;

    example.Serialize(archive);
}

void loop() {
}
