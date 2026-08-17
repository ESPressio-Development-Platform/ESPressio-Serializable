#include <Arduino.h>
#include <ESPressio_Serializable.hpp>

using namespace ESPressio;

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
