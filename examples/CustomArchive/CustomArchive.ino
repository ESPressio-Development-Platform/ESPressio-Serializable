#include <Arduino.h>
#include <ESPressio_Serializable.hpp>

using namespace ESPressio;

/*
 * Demonstrates the intended extension point.
 *
 * A real JSON archive could forward these calls to ArduinoJson without
 * requiring ESPressio Serializable itself to depend on ArduinoJson.
 */
/**
 * ESPressio Memory Audit
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 0 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
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

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<Example>) [0 bytes dynamic allocation]
 * Members:
 * - _ratio (float): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: sizeof(Serializable::Serializable<Example>) + 4 bytes known members [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
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
