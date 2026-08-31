#include <cassert>
#include <cstdint>
#include <string_view>

#include "ESPressio_BinaryArchive.hpp"
#include "ESPressio_SerializationNode.hpp"

using namespace ESPressio::Serializable;

int main() {
    {
        SerializationNode root(SerializationNodeType::Object);
        SerializationString retainedName;
        retainedName.assign(
            "property-name-long-enough-to-own-storage",
            40
        );
        const char* const originalStorage = retainedName.data();

        SerializationNode value(SerializationNodeType::UnsignedInteger);
        value.UnsignedIntegerValue() = 42;
        root.SetOwned(std::move(retainedName), std::move(value));

        assert(root.ObjectChildren().size() == 1);
        assert(root.ObjectChildren().front().first.data() == originalStorage);
        assert(root.ObjectChildren().front().first ==
            "property-name-long-enough-to-own-storage");
        assert(root.ObjectChildren().front().second.UnsignedIntegerValue() == 42);
    }

    {
        BinaryArchive source;
        SerializationNode child(SerializationNodeType::String);
        child.StringValue().assign("payload", 7);
        source.GetRootNode().Set("alpha", std::move(child));

        SerializationNode array(SerializationNodeType::Array);
        for (uint64_t index = 0; index < 8; ++index) {
            SerializationNode item(SerializationNodeType::UnsignedInteger);
            item.UnsignedIntegerValue() = index;
            array.Append(std::move(item));
        }
        source.GetRootNode().Set("items", std::move(array));

        const auto bytes = source.GetData();
        BinaryArchive decoded(bytes);
        assert(decoded.IsValid());

        const auto* alpha = decoded.GetRootNode().Find(std::string_view("alpha"));
        assert(alpha != nullptr);
        assert(alpha->StringValue() == "payload");

        const auto* items = decoded.GetRootNode().Find(std::string_view("items"));
        assert(items != nullptr);
        assert(items->ArrayChildren().size() == 8);
        for (uint64_t index = 0; index < 8; ++index) {
            assert(items->ArrayChildren()[index].UnsignedIntegerValue() == index);
        }
    }

    return 0;
}
