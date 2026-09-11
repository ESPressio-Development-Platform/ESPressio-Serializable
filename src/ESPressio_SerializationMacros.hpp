#pragma once
#include <cstdint>
#include <tuple>
#include <utility>
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

/// <summary>Declares the sole property tuple used by archives and static bounded schema derivation.</summary>
/// <remarks>No metadata allocation or lazy shared ownership is performed. Bounded fields permit constant evaluation;
/// general-purpose fields with dynamic defaults may still construct their tuple at runtime.</remarks>
#define ESPRESSIO_SERIALIZABLE_PROPERTIES(...) \
    public: \
        template<class ESPressioMetadataEvaluation = void> \
        static constexpr auto GetSerializableProperties() { \
            return std::make_tuple(__VA_ARGS__); \
        }

#define ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(Version) \
    public: \
        static constexpr uint32_t GetSerializableSchemaVersion() { \
            return static_cast<uint32_t>(Version); \
        }

#define ESPRESSIO_PROPERTY_NAMELESS(Member) \
    ::ESPressio::Serializable::MakeSerializationProperty(nullptr, &ESPressioSerializableSelf::Member)
