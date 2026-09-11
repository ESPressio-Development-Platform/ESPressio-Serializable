#include <ESPressio_SerializationTraits.hpp>
#include <ESPressio_SerializationMacros.hpp>
struct Missing {
    int Value{};
    ESPRESSIO_SERIALIZABLE_TYPE(Missing)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("value",Value))
};
constexpr auto maximum=ESPressio::Serializable::MaximumSerializedSize<Missing,ESPressio::Serializable::CBOR>;
