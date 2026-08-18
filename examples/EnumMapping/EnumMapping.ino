#include <Arduino.h>
#include <ESPressio_Serializable_JSON.hpp>

using namespace ESPressio;

enum class Mode : uint8_t { Off, Standby, Active };

ESPRESSIO_ENUM_MAPPING(
    Mode,
    ESPRESSIO_ENUM_VALUE(Mode::Off, "off"),
    ESPRESSIO_ENUM_VALUE(Mode::Standby, "standby"),
    ESPRESSIO_ENUM_VALUE(Mode::Active, "active")
)

class Settings final : public Serializable::Serializable<Settings> {
    ESPRESSIO_SERIALIZABLE_TYPE(Settings)
    private:
        Mode _mode = Mode::Active;
    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("mode", _mode)
        )
};

void setup() { Serial.begin(115200); }
void loop() {}
