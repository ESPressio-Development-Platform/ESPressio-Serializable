#include <Arduino.h>
#include <ESPressio_Serializable.hpp>

using namespace ESPressio;

static bool IsEven(const int32_t& value) {
    return (value % 2) == 0;
}

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members:
 * - _batchSize (int32_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 8 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class Settings final : public Serializable::Serializable<Settings> {
    ESPRESSIO_SERIALIZABLE_TYPE(Settings)

    private:
        uint16_t _port = 0;
        int32_t _batchSize = 0;

    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("port", _port)
                .Default(static_cast<uint16_t>(80))
                .Range(static_cast<uint16_t>(1), static_cast<uint16_t>(65535)),
            ESPRESSIO_PROPERTY("batchSize", _batchSize)
                .Default(10)
                .Validate(&IsEven)
        )
};

void setup() { Serial.begin(115200); }
void loop() {}
