#include <Arduino.h>
#include <array>
#include <vector>

#include <ESPressio_Serializable_CBOR.hpp>

using namespace ESPressio;

/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
enum
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
class SensorMode : uint8_t {
    Disabled = 0,
    Active = 1
};

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<Position>) [0 bytes dynamic allocation]
 * Members:
 * - _y (float): 4 bytes [0 bytes dynamic allocation]
 * - _z (float): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: sizeof(Serializable::Serializable<Position>) + 8 bytes known members [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class Position final
    : public Serializable::Serializable<Position> {

    ESPRESSIO_SERIALIZABLE_TYPE(Position)

    private:
        float _x = 1.0f;
        float _y = 2.0f;
        float _z = 3.0f;

    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("x", _x),
            ESPRESSIO_PROPERTY("y", _y),
            ESPRESSIO_PROPERTY("z", _z)
        )
};

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<Sensor>) [0 bytes dynamic allocation]
 * Members:
 * - _mode (SensorMode): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: sizeof(Serializable::Serializable<Sensor>) + 1 bytes known members [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class Sensor final
    : public Serializable::Serializable<Sensor> {

    ESPRESSIO_SERIALIZABLE_TYPE(Sensor)

    private:
        uint32_t _id = 0;
        SensorMode _mode = SensorMode::Active;

    public:
        Sensor() = default;

        explicit Sensor(uint32_t id)
            : _id(id) {
        }

        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("id", _id),
            ESPRESSIO_PROPERTY("mode", _mode)
        )
};

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<Device>) [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: sizeof(Serializable::Serializable<Device>) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class Device final
    : public Serializable::Serializable<Device> {

    ESPRESSIO_SERIALIZABLE_TYPE(Device)

    private:
        Position _position;
        std::array<uint16_t, 3> _calibration {
            100, 200, 300
        };
        std::vector<Sensor> _sensors {
            Sensor(1),
            Sensor(2)
        };

    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("position", _position),
            ESPRESSIO_PROPERTY("calibration", _calibration),
            ESPRESSIO_PROPERTY("sensors", _sensors)
        )
};

void setup() {
    Serial.begin(115200);

    Device device;
    Serializable::CborArchive archive;

    // Position, std::array, std::vector, Sensor and SensorMode
    // are all traversed by the common core implementation.
    device.Serialize(archive);

    const auto payload = archive.GetData();

    Serial.printf(
        "Nested CBOR payload: %u bytes\n",
        static_cast<unsigned>(payload.size())
    );

    Serializable::CborArchive restoredArchive(
        payload
    );

    Device restored;
    restored.Deserialize(restoredArchive);
}

void loop() {
}
