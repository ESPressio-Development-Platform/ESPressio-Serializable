#include <Arduino.h>
#include <array>
#include <vector>

#include <ESPressio_Serializable_CBOR.hpp>

using namespace ESPressio;

enum class SensorMode : uint8_t {
    Disabled = 0,
    Active = 1
};

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
