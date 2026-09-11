#include <ESPressio_SerializationTraits.hpp>
#include <ESPressio_SerializationMacros.hpp>
struct Value {
    int Number{};
    ESPRESSIO_SERIALIZABLE_TYPE(Value)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("number",Number))
};
struct UnprovenFormat {};
constexpr auto maximum=ESPressio::Serializable::MaximumSerializedSize<Value,UnprovenFormat>;
