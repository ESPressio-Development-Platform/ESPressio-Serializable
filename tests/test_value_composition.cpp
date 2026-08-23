#include <cassert>
#include <string>
#include <vector>

#include <ESPressio_Serializable.hpp>
#include <ESPressio_Serializable_Binary.hpp>

using namespace ESPressio::Serializable;

struct Child final : Serializable<Child> {
    ESPRESSIO_SERIALIZABLE_TYPE(Child)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    int Value = 0;
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("value", Value))
};

struct Parent final : Serializable<Parent> {
    ESPRESSIO_SERIALIZABLE_TYPE(Parent)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    Child Nested{};
    std::vector<Child> Children;
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY("nested", Nested),
        ESPRESSIO_PROPERTY("children", Children)
    )
};

int main() {
    static_assert(std::is_destructible<Child>::value, "Serializable values must be publicly destructible");
    static_assert(std::is_destructible<Parent>::value, "Composed Serializable values must be publicly destructible");

    Parent source;
    source.Nested.Value = 7;
    source.Children.push_back(Child{});
    source.Children.back().Value = 11;
    source.Children.push_back(Child{});
    source.Children.back().Value = 13;

    BinaryArchive archive;
    source.Serialize(archive);
    const auto data = archive.GetData();
    assert(!data.empty());

    BinaryArchive input;
    assert(input.Load(data.data(), data.size()));
    Parent restored;
    const auto result = restored.DeserializeDetailed(input);
    assert(result.Success());
    assert(restored.Nested.Value == 7);
    assert(restored.Children.size() == 2);
    assert(restored.Children[0].Value == 11);
    assert(restored.Children[1].Value == 13);

    return 0;
}
