#include <algorithm>
#include <cassert>
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include <ESPressio_Allocator.hpp>
#include <ESPressio_BinaryArchive.hpp>
#include <ESPressio_DirectBinaryArchive.hpp>
#include <ESPressio_Serializable.hpp>

using namespace ESPressio;

enum class DirectMode : uint8_t { Off = 0, On = 1 };

ESPRESSIO_ENUM_MAPPING(
    DirectMode,
    ESPRESSIO_ENUM_VALUE(DirectMode::Off, "off"),
    ESPRESSIO_ENUM_VALUE(DirectMode::On, "on")
)

class DirectChild final : public Serializable::Serializable<DirectChild> {
    ESPRESSIO_SERIALIZABLE_TYPE(DirectChild)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
private:
    int32_t _value = 17;
public:
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("value", _value))
    int32_t GetValue() const { return _value; }
};

class DirectPayload final : public Serializable::Serializable<DirectPayload> {
    ESPRESSIO_SERIALIZABLE_TYPE(DirectPayload)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
private:
    uint32_t _id = 42;
    std::string _name = "direct-binary";
    DirectMode _mode = DirectMode::On;
    DirectChild _child;
    std::vector<int32_t> _values {1,2,3,4};
    std::optional<uint16_t> _optional = 99;
    std::map<std::string, int32_t> _mapping {{"alpha",1},{"beta",2}};
public:
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY("id", _id),
        ESPRESSIO_PROPERTY("name", _name),
        ESPRESSIO_PROPERTY("mode", _mode),
        ESPRESSIO_PROPERTY("child", _child),
        ESPRESSIO_PROPERTY("values", _values),
        ESPRESSIO_PROPERTY("optional", _optional),
        ESPRESSIO_PROPERTY("mapping", _mapping)
    )
    uint32_t GetId() const { return _id; }
    const std::string& GetName() const { return _name; }
    DirectMode GetMode() const { return _mode; }
    const DirectChild& GetChild() const { return _child; }
    const std::vector<int32_t>& GetValues() const { return _values; }
    const std::optional<uint16_t>& GetOptional() const { return _optional; }
    const std::map<std::string, int32_t>& GetMapping() const { return _mapping; }
};

int main() {
    DirectPayload original;

    Serializable::BinaryArchive archive;
    original.Serialize(archive);
    const auto archiveBytes = archive.GetData();

    std::vector<uint8_t> directBytes;
    assert(Serializable::SerializeDirectBinary(original, directBytes));

    assert(directBytes.size() == archiveBytes.size());
    assert(std::equal(directBytes.begin(), directBytes.end(), archiveBytes.begin()));

    Serializable::SerializationBuffer<uint8_t> policyBytes;
    assert(Serializable::SerializeDirectBinary(original, policyBytes));
    assert(policyBytes.size() == directBytes.size());
    assert(std::equal(policyBytes.begin(), policyBytes.end(), directBytes.begin()));

    DirectPayload restored;
    const auto directResult = Serializable::DeserializeDirectBinary(
        policyBytes.data(), policyBytes.size(), restored
    );

    assert(directResult.Success());
    assert(restored.GetId() == 42);
    assert(restored.GetName() == "direct-binary");
    assert(restored.GetMode() == DirectMode::On);
    assert(restored.GetChild().GetValue() == 17);
    assert(restored.GetValues().size() == 4);
    assert(restored.GetValues()[3] == 4);
    assert(restored.GetOptional().has_value());
    assert(*restored.GetOptional() == 99);
    assert(restored.GetMapping().at("beta") == 2);

    std::vector<uint8_t> prefixed {0xAA, 0x55};
    assert(Serializable::AppendDirectBinary(original, prefixed));
    assert(prefixed.size() == directBytes.size() + 2);
    assert(prefixed[0] == 0xAA);
    assert(prefixed[1] == 0x55);
    assert(std::equal(directBytes.begin(), directBytes.end(), prefixed.begin() + 2));

    Serializable::SerializationBuffer<uint8_t> policyPrefixed {0xAA, 0x55};
    assert(Serializable::AppendDirectBinary(original, policyPrefixed));
    assert(policyPrefixed.size() == directBytes.size() + 2);
    assert(std::equal(
        policyPrefixed.begin(),
        policyPrefixed.end(),
        prefixed.begin()
    ));

    auto malformed = directBytes;
    malformed.pop_back();
    DirectPayload invalid;
    assert(!Serializable::DeserializeDirectBinary(
        malformed.data(), malformed.size(), invalid
    ).Success());

    return 0;
}
