#include <Arduino.h>
#include <ESPressio_Serializable.hpp>

using namespace ESPressio;

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<MotorConfiguration>) [0 bytes dynamic allocation]
 * Members:
 * - _maximumSpeed (int32_t): 4 bytes [0 bytes dynamic allocation]
 * - _reversed (bool): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: sizeof(Serializable::Serializable<MotorConfiguration>) + 5 bytes known members [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class MotorConfiguration final
    : public Serializable::Serializable<MotorConfiguration> {

    ESPRESSIO_SERIALIZABLE_TYPE(MotorConfiguration)

    private:
        int32_t _minimumSpeed = 0;
        int32_t _maximumSpeed = 100;
        bool _reversed = false;

    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("minimumSpeed", _minimumSpeed),
            ESPRESSIO_PROPERTY("maximumSpeed", _maximumSpeed),
            ESPRESSIO_PROPERTY("reversed", _reversed)
        )

        int32_t GetMinimumSpeed() const { return _minimumSpeed; }
        int32_t GetMaximumSpeed() const { return _maximumSpeed; }
        bool IsReversed() const { return _reversed; }
};

void setup() {
    Serial.begin(115200);

    Serializable::KeyValueArchive persisted;
    persisted.Write("minimumSpeed", 10);
    persisted.Write("maximumSpeed", 240);
    persisted.Write("reversed", true);

    MotorConfiguration configuration;
    configuration.Deserialize(persisted);

    Serial.printf(
        "minimum=%ld maximum=%ld reversed=%s\n",
        static_cast<long>(configuration.GetMinimumSpeed()),
        static_cast<long>(configuration.GetMaximumSpeed()),
        configuration.IsReversed() ? "true" : "false"
    );
}

void loop() {
}
