#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>

#include <ESPressio_Serializable.hpp>

using namespace ESPressio;

class TestConfiguration final
    : public Serializable::Serializable<TestConfiguration> {

    ESPRESSIO_SERIALIZABLE_TYPE(TestConfiguration)

    private:
        int32_t _port = 80;
        bool _enabled = true;
        float _threshold = 0.25f;

    public:
        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("port", _port),
            ESPRESSIO_PROPERTY("enabled", _enabled),
            ESPRESSIO_PROPERTY("threshold", _threshold)
        )

        int32_t GetPort() const { return _port; }
        bool IsEnabled() const { return _enabled; }
        float GetThreshold() const { return _threshold; }
};

int main() {
    static_assert(
        Serializable::IsSerializable<TestConfiguration>,
        "TestConfiguration must expose serializable properties."
    );

    TestConfiguration original;
    Serializable::KeyValueArchive archive;

    original.Serialize(archive);

    assert(archive.GetEntries().size() == 3);

    Serializable::KeyValueArchive replacement;
    replacement.Write("port", 443);
    replacement.Write("enabled", false);
    replacement.Write("threshold", 0.75f);

    TestConfiguration restored;
    restored.Deserialize(replacement);

    assert(restored.GetPort() == 443);
    assert(restored.IsEnabled() == false);
    assert(restored.GetThreshold() > 0.749f);
    assert(restored.GetThreshold() < 0.751f);

    std::cout << "All ESPressio Serializable tests passed.\n";
    return 0;
}
