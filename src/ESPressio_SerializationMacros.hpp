#pragma once
#include <cstdint>
#include <tuple>
#include <utility>
#include <ESPressio_Memory.hpp>
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

/// <summary>Declares the stable serializable property metadata for a type.</summary>
/// <remarks>The generated accessor stores the immutable descriptor tuple once in ESPressio System ExternalPreferred memory and returns only a lightweight tuple of references on each call.</remarks>
#define ESPRESSIO_SERIALIZABLE_PROPERTIES(...) \
    public: \
        static auto GetSerializableProperties() { \
            using ESPressioSerializablePropertiesStorage = \
                decltype(std::make_tuple(__VA_ARGS__)); \
            static const auto storage = \
                ::ESPressio::System::Memory::MakeShared< \
                    ESPressioSerializablePropertiesStorage, \
                    ::ESPressio::System::Memory::MemoryPolicy::ExternalPreferred \
                >(__VA_ARGS__); \
            return std::apply( \
                [](const auto&... property) { \
                    return std::forward_as_tuple(property...); \
                }, \
                *storage \
            ); \
        }

#define ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(Version) \
    public: \
        static constexpr uint32_t GetSerializableSchemaVersion() { \
            return static_cast<uint32_t>(Version); \
        }

#define ESPRESSIO_PROPERTY_NAMELESS(Member) \
    ::ESPressio::Serializable::MakeSerializationProperty(nullptr, &ESPressioSerializableSelf::Member)
