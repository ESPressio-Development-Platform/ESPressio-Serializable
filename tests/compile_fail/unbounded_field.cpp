#include <ESPressio_SerializationTraits.hpp>
#include <ESPressio_SerializationMacros.hpp>
#include <string>
struct Unbounded {
    std::string Text;
    ESPRESSIO_SERIALIZABLE_TYPE(Unbounded)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("text",Text))
};
constexpr auto maximum=ESPressio::Serializable::MaximumSerializedSize<Unbounded,ESPressio::Serializable::DirectBinary>;
