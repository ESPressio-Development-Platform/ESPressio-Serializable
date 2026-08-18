#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <ESPressio_Serializable.hpp>
#include <ESPressio_BinaryArchive.hpp>
#include <ESPressio_CborArchive.hpp>

using namespace ESPressio;

enum class TestMode : uint8_t { Off = 0, On = 1 };

class Position final
    : public Serializable::Serializable<Position> {
    ESPRESSIO_SERIALIZABLE_TYPE(Position)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    private:
        float _x = 1.5f;
        float _y = 2.5f;
    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("x", _x),
            ESPRESSIO_PROPERTY("y", _y)
        )
        float GetX() const { return _x; }
        float GetY() const { return _y; }
};

class Sensor final
    : public Serializable::Serializable<Sensor> {
    ESPRESSIO_SERIALIZABLE_TYPE(Sensor)
    private:
        uint32_t _id = 0;
        TestMode _mode = TestMode::On;
    public:
        Sensor() = default;
        explicit Sensor(uint32_t id) : _id(id) {}
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY_REQUIRED("id", _id),
            ESPRESSIO_PROPERTY("mode", _mode)
        )
        uint32_t GetId() const { return _id; }
        TestMode GetMode() const { return _mode; }
};

class TestConfiguration final
    : public Serializable::Serializable<TestConfiguration> {
    ESPRESSIO_SERIALIZABLE_TYPE(TestConfiguration)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(2)
    private:
        int32_t _port = 443;
        bool _enabled = true;
        std::string _name = "device-A";
        Position _position;
        std::array<uint16_t, 3> _calibration { 10, 20, 30 };
        std::vector<Sensor> _sensors { Sensor(7), Sensor(8), Sensor(9) };
        std::optional<int32_t> _optionalValue = 123;
        std::optional<int32_t> _emptyOptional;
        std::map<std::string, int32_t> _namedValues {
            {"one", 1}, {"two", 2}
        };
        std::unordered_map<uint32_t, Sensor> _sensorMap {
            {10, Sensor(10)}, {20, Sensor(20)}
        };
        uint32_t _readOnlyId = 999;
    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("port", _port).Alias("oldPort"),
            ESPRESSIO_PROPERTY("enabled", _enabled),
            ESPRESSIO_PROPERTY("name", _name).Alias("deviceName"),
            ESPRESSIO_PROPERTY("position", _position),
            ESPRESSIO_PROPERTY("calibration", _calibration),
            ESPRESSIO_PROPERTY("sensors", _sensors),
            ESPRESSIO_PROPERTY("optionalValue", _optionalValue),
            ESPRESSIO_PROPERTY("emptyOptional", _emptyOptional),
            ESPRESSIO_PROPERTY("namedValues", _namedValues),
            ESPRESSIO_PROPERTY("sensorMap", _sensorMap),
            ESPRESSIO_PROPERTY_READONLY("readOnlyId", _readOnlyId)
        )

        static bool Migrate(
            Serializable::SerializationNode& node,
            uint32_t fromVersion,
            uint32_t toVersion
        ) {
            if (fromVersion == 1 && toVersion == 2) {
                auto* oldNode = node.Find("deviceName");
                if (oldNode != nullptr && node.Find("name") == nullptr) {
                    node.Set("name", *oldNode);
                }

                node.Set(
                    "__schemaVersion",
                    Serializable::Detail::ToNode(
                        static_cast<uint32_t>(2)
                    )
                );
                return true;
            }
            return false;
        }

        int32_t GetPort() const { return _port; }
        bool IsEnabled() const { return _enabled; }
        const std::string& GetName() const { return _name; }
        const Position& GetPosition() const { return _position; }
        const auto& GetCalibration() const { return _calibration; }
        const auto& GetSensors() const { return _sensors; }
        const auto& GetOptionalValue() const { return _optionalValue; }
        const auto& GetEmptyOptional() const { return _emptyOptional; }
        const auto& GetNamedValues() const { return _namedValues; }
        const auto& GetSensorMap() const { return _sensorMap; }
        uint32_t GetReadOnlyId() const { return _readOnlyId; }
};

template<typename TArchive>
void TestRoundTrip() {
    TestConfiguration original;
    TArchive output;
    original.Serialize(output);

    const auto data = output.GetData();
    assert(!data.empty());

    TArchive input;
    assert(input.Load(data));
    assert(input.IsValid());

    TestConfiguration restored;
    assert(restored.Deserialize(input));

    assert(restored.GetPort() == 443);
    assert(restored.IsEnabled());
    assert(restored.GetName() == "device-A");
    assert(restored.GetPosition().GetX() > 1.49f);
    assert(restored.GetPosition().GetX() < 1.51f);
    assert(restored.GetCalibration()[2] == 30);
    assert(restored.GetSensors().size() == 3);
    assert(restored.GetSensors()[1].GetId() == 8);
    assert(restored.GetOptionalValue().has_value());
    assert(*restored.GetOptionalValue() == 123);
    assert(!restored.GetEmptyOptional().has_value());
    assert(restored.GetNamedValues().at("two") == 2);
    assert(restored.GetSensorMap().at(20).GetId() == 20);
    assert(restored.GetReadOnlyId() == 999);
}

void TestAliasAndMigration() {
    Serializable::TreeArchive archive;
    archive.Write("__schemaVersion", static_cast<uint32_t>(1));
    archive.Write("oldPort", static_cast<int32_t>(8080));
    archive.Write("deviceName", std::string("legacy"));

    TestConfiguration restored;
    assert(restored.Deserialize(archive));
    assert(restored.GetPort() == 8080);
    assert(restored.GetName() == "legacy");
}

void TestRequiredProperty() {
    Serializable::TreeArchive archive;
    Sensor sensor;
    assert(!sensor.Deserialize(archive));
}

int main() {
    static_assert(
        Serializable::IsSerializable<TestConfiguration>,
        "TestConfiguration must expose serializable properties."
    );

    TestRoundTrip<Serializable::BinaryArchive>();
    TestRoundTrip<Serializable::CborArchive>();
    TestAliasAndMigration();
    TestRequiredProperty();

    std::cout << "ESPressio Serializable v0.4 tests passed.\n";
    return 0;
}
