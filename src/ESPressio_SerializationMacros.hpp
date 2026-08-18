#pragma once
#include <cstdint>
#include <tuple>
#include "ESPressio_SerializationProperty.hpp"

#define ESPRESSIO_SERIALIZABLE_TYPE(Type) \
    private: \
        using ESPressioSerializableSelf = Type;

#define ESPRESSIO_PROPERTY(Name, Member) \
    ::ESPressio::Serializable::MakeSerializationProperty( \
        Name, \
        &ESPressioSerializableSelf::Member \
    )

#define ESPRESSIO_PROPERTY_REQUIRED(Name, Member) \
    ESPRESSIO_PROPERTY(Name, Member).Required()

#define ESPRESSIO_PROPERTY_READONLY(Name, Member) \
    ESPRESSIO_PROPERTY(Name, Member).ReadOnly()

#define ESPRESSIO_PROPERTY_SENSITIVE(Name, Member) \
    ESPRESSIO_PROPERTY(Name, Member).Sensitive()

#define ESPRESSIO_SERIALIZABLE_PROPERTIES(...) \
    public: \
        static auto GetSerializableProperties() { \
            return std::make_tuple(__VA_ARGS__); \
        }

#define ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(Version) \
    public: \
        static constexpr uint32_t GetSerializableSchemaVersion() { \
            return static_cast<uint32_t>(Version); \
        }

#define ESPRESSIO_PROPERTY_NAMELESS(Member) \
    ::ESPressio::Serializable::MakeSerializationProperty(nullptr, &ESPressioSerializableSelf::Member)
