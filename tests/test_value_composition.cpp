#include <cassert>
#include <string>
#include <type_traits>
#include <vector>

#include <ESPressio_Serializable.hpp>
#include <ESPressio_Serializable_Binary.hpp>

using namespace ESPressio::Serializable;

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (standalone empty object occupies 1 byte; an eligible empty base may be optimized to 0 bytes).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
struct Child final : Serializable<Child> {
    ESPRESSIO_SERIALIZABLE_TYPE(Child)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    int Value = 0;
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("value", Value))
};

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members:
 * - Children (std::vector<Child>): 12 bytes [Capacity * (1 bytes) element storage]
 * Total Memory: 16 bytes [Children: Capacity * (1 bytes) element storage]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
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

/**
 * ESPressio Memory Audit
 * Members:
 * - Nested (Child): 1 bytes [0 bytes dynamic allocation]
 * - Children (std::vector<Child>): 12 bytes [Capacity * (1 bytes) element storage]
 * Total Memory: 16 bytes [Children: Capacity * (1 bytes) element storage]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
struct OrdinaryAggregate {
    Child Nested{};
    std::vector<Child> Children;
};

int main() {
    static_assert(std::is_default_constructible<Child>::value,
                  "Serializable values must remain default constructible");
    static_assert(std::is_destructible<Child>::value,
                  "Serializable values must be publicly destructible");
    static_assert(std::is_copy_constructible<Child>::value,
                  "Serializable values must remain copy constructible");
    static_assert(std::is_move_constructible<Child>::value,
                  "Serializable values must remain move constructible");
    static_assert(std::is_destructible<Parent>::value,
                  "Composed Serializable values must be publicly destructible");
    static_assert(std::is_destructible<OrdinaryAggregate>::value,
                  "Serializable values must be usable in ordinary aggregates");

    OrdinaryAggregate aggregate;
    aggregate.Nested.Value = 3;
    aggregate.Children.emplace_back();
    aggregate.Children.back().Value = 5;
    assert(aggregate.Nested.Value == 3);
    assert(aggregate.Children.front().Value == 5);

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
