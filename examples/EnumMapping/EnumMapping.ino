#include <Arduino.h>
#include <ESPressio_Serializable_JSON.hpp>

using namespace ESPressio;

/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
enum
class Mode : uint8_t { Off, Standby, Active };

ESPRESSIO_ENUM_MAPPING(
    Mode,
    ESPRESSIO_ENUM_VALUE(Mode::Off, "off"),
    ESPRESSIO_ENUM_VALUE(Mode::Standby, "standby"),
    ESPRESSIO_ENUM_VALUE(Mode::Active, "active")
)

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (standalone empty object occupies 1 byte; an eligible empty base may be optimized to 0 bytes).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
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
