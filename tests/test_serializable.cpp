#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>
#include <map>
#include <set>
#include <unordered_set>
#include <list>
#include <deque>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include <ESPressio_Serializable.hpp>
#include <ESPressio_BinaryArchive.hpp>
#include <ESPressio_CborArchive.hpp>

using namespace ESPressio;

/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
enum
class TestMode : uint8_t { Off = 0, On = 1 };

ESPRESSIO_ENUM_MAPPING(
    TestMode,
    ESPRESSIO_ENUM_VALUE(TestMode::Off, "off"),
    ESPRESSIO_ENUM_VALUE(TestMode::On, "on")
)

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members:
 * - _y (float): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 8 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
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

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members:
 * - _mode (TestMode): 1 bytes [0 bytes dynamic allocation]
 * Total Memory: 2 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
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

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members:
 * - _enabled (bool): 1 bytes [0 bytes dynamic allocation]
 * - _name (std::string): 24 bytes [Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * - _position (Position): 8 bytes [0 bytes dynamic allocation]
 * - _calibration (std::array<uint16_t, 3>): 6 bytes [0 bytes dynamic allocation]
 * - _sensors (std::vector<Sensor>): 12 bytes [Capacity * (2 bytes) element storage]
 * - _optionalValue (std::optional<int32_t>): 8 bytes [0 bytes dynamic allocation]
 * - _emptyOptional (std::optional<int32_t>): 8 bytes [0 bytes dynamic allocation]
 * - _namedValues (std::map<std::string, int32_t>): 24 bytes [N * (16 bytes red-black-tree node linkage + 28 bytes value); key/value: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * - _sensorMap (std::unordered_map<uint32_t, Sensor>): 28 bytes [BucketCount * 4 bytes + N * (hash-node/link overhead + 8 bytes value)]
 * - _readOnlyId (uint32_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 128 bytes [_name: Capacity + 1 bytes when capacity exceeds 15-byte SSO; _sensors: Capacity * (2 bytes) element storage; _namedValues: N * (16 bytes red-black-tree node linkage + 28 bytes value); _namedValues: key/value: Capacity + 1 bytes when capacity exceeds 15-byte SSO; _sensorMap: BucketCount * 4 bytes + N * (hash-node/link overhead + 8 bytes value)]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
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


bool ValidateEven(const int32_t& value) {
    return (value % 2) == 0;
}

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members:
 * - _even (int32_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 8 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class ValidationConfiguration final
    : public Serializable::Serializable<ValidationConfiguration> {
    ESPRESSIO_SERIALIZABLE_TYPE(ValidationConfiguration)
    private:
        int32_t _port = 1;
        int32_t _even = 0;
    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("port", _port).Default(80).Range(1, 65535),
            ESPRESSIO_PROPERTY("even", _even).Validate(&ValidateEven)
        )
        int32_t GetPort() const { return _port; }
        int32_t GetEven() const { return _even; }
};

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members:
 * - _unordered (std::unordered_set<std::string>): 28 bytes [BucketCount * 4 bytes + N * (hash-node/link overhead + 24 bytes value); element: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * - _list (std::list<int32_t>): 12 bytes [N * (8 bytes node links + 4 bytes)]
 * - _deque (std::deque<int32_t>): 40 bytes [implementation blocks containing N * (4 bytes) plus block-map pointers]
 * Total Memory: 84 bytes [_unordered: BucketCount * 4 bytes + N * (hash-node/link overhead + 24 bytes value); _unordered: element: Capacity + 1 bytes when capacity exceeds 15-byte SSO; _list: N * (8 bytes node links + 4 bytes); _deque: implementation blocks containing N * (4 bytes) plus block-map pointers]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class CollectionConfiguration final
    : public Serializable::Serializable<CollectionConfiguration> {
    ESPRESSIO_SERIALIZABLE_TYPE(CollectionConfiguration)
    private:
        std::set<int32_t> _set {3, 1, 2};
        std::unordered_set<std::string> _unordered {"a", "b"};
        std::list<int32_t> _list {4, 5, 6};
        std::deque<int32_t> _deque {7, 8, 9};
    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("set", _set),
            ESPRESSIO_PROPERTY("unordered", _unordered),
            ESPRESSIO_PROPERTY("list", _list),
            ESPRESSIO_PROPERTY("deque", _deque)
        )
        const auto& GetSet() const { return _set; }
        const auto& GetUnordered() const { return _unordered; }
        const auto& GetList() const { return _list; }
        const auto& GetDeque() const { return _deque; }
};

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members:
 * - _secret (std::string): 24 bytes [Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Total Memory: 28 bytes [_secret: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class SensitiveConfiguration final
    : public Serializable::Serializable<SensitiveConfiguration> {
    ESPRESSIO_SERIALIZABLE_TYPE(SensitiveConfiguration)
    private:
        std::string _public = "hello";
        std::string _secret = "password";
    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("public", _public),
            ESPRESSIO_PROPERTY("secret", _secret).Sensitive()
        )
};

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (standalone empty object occupies 1 byte; an eligible empty base may be optimized to 0 bytes).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class NarrowConfiguration final
    : public Serializable::Serializable<NarrowConfiguration> {
    ESPRESSIO_SERIALIZABLE_TYPE(NarrowConfiguration)
    private:
        int8_t _value = 0;
    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("value", _value)
        )
};

void TestDefaultsAndValidation() {
    {
        Serializable::TreeArchive archive;
        archive.Write("even", static_cast<int32_t>(4));
        ValidationConfiguration value;
        assert(value.Deserialize(archive));
        assert(value.GetPort() == 80);
        assert(value.GetEven() == 4);
    }

    {
        Serializable::TreeArchive archive;
        archive.Write("port", static_cast<int32_t>(70000));
        archive.Write("even", static_cast<int32_t>(4));
        ValidationConfiguration value;
        assert(!value.Deserialize(archive));
    }

    {
        Serializable::TreeArchive archive;
        archive.Write("port", static_cast<int32_t>(80));
        archive.Write("even", static_cast<int32_t>(3));
        ValidationConfiguration value;
        assert(!value.Deserialize(archive));
    }
}

void TestAdditionalCollections() {
    CollectionConfiguration original;
    Serializable::CborArchive output;
    original.Serialize(output);

    Serializable::CborArchive input(output.GetData());
    CollectionConfiguration restored;
    assert(restored.Deserialize(input));
    assert(restored.GetSet().size() == 3);
    assert(restored.GetUnordered().count("a") == 1);
    assert(restored.GetList().size() == 3);
    assert(restored.GetDeque().back() == 9);
}

void TestEnumMapping() {
    const auto node = Serializable::Detail::ToNode(TestMode::On);
    assert(node.GetType() == Serializable::SerializationNodeType::String);
    assert(node.StringValue() == "on");

    TestMode value = TestMode::Off;
    assert(Serializable::Detail::FromNode(node, value));
    assert(value == TestMode::On);
}

void TestSensitivePolicies() {
    SensitiveConfiguration value;

    Serializable::TreeArchive redacted;
    redacted.Policy()
        .Sensitive(Serializable::SensitivePropertyPolicy::Redact)
        .RedactionText("***");
    value.Serialize(redacted);
    const auto* secret = redacted.GetNode().Find("secret");
    assert(secret != nullptr);
    assert(secret->StringValue() == "***");

    Serializable::TreeArchive omitted;
    omitted.Policy().Sensitive(Serializable::SensitivePropertyPolicy::Omit);
    value.Serialize(omitted);
    assert(omitted.GetNode().Find("secret") == nullptr);
}

void TestMigrationHelpers() {
    Serializable::SerializationNode root(Serializable::SerializationNodeType::Object);
    root.Set("old", Serializable::Detail::ToNode(std::string("value")));
    assert(Serializable::Migration::Rename(root, "old", "new"));
    assert(root.Find("old") == nullptr);
    assert(root.Find("new") != nullptr);

    Serializable::SerializationNode nested(Serializable::SerializationNodeType::Object);
    root.Set("nested", nested);
    auto* destination = root.Find("nested");
    assert(destination != nullptr);
    assert(Serializable::Migration::Move(root, "new", *destination, "moved"));
    assert(destination->Find("moved") != nullptr);
    assert(Serializable::Migration::Remove(*destination, "moved"));
}

void TestNumericNarrowing() {
    Serializable::TreeArchive archive;
    archive.Write("value", static_cast<uint64_t>(1000));
    NarrowConfiguration value;
    assert(!value.Deserialize(archive));
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
    TestDefaultsAndValidation();
    TestAdditionalCollections();
    TestEnumMapping();
    TestSensitivePolicies();
    TestMigrationHelpers();
    TestNumericNarrowing();

    std::cout << "ESPressio Serializable v0.5 tests passed.\n";
    return 0;
}
