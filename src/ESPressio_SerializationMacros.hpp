#pragma once

#include <tuple>
#include "ESPressio_SerializationProperty.hpp"

/*
 * C++ has no portable user-defined runtime reflection/annotation facility
 * suitable for Arduino-class toolchains. These macros provide a small,
 * declarative compile-time schema while keeping ordinary C++ members.
 *
 * Usage:
 *
 * class Settings : public Serializable::Serializable<Settings> {
 *     ESPRESSIO_SERIALIZABLE_TYPE(Settings)
 *
 *   private:
 *     int _port = 80;
 *
 *   public:
 *     ESPRESSIO_SERIALIZABLE_PROPERTIES(
 *         ESPRESSIO_PROPERTY("port", _port)
 *     )
 * };
 */

#define ESPRESSIO_SERIALIZABLE_TYPE(Type) \
    private: \
        using ESPressioSerializableSelf = Type;

#define ESPRESSIO_PROPERTY(Name, Member) \
    ::ESPressio::Serializable::MakeSerializationProperty( \
        Name, \
        &ESPressioSerializableSelf::Member \
    )

#define ESPRESSIO_SERIALIZABLE_PROPERTIES(...) \
    public: \
        static constexpr auto GetSerializableProperties() { \
            return std::make_tuple(__VA_ARGS__); \
        }
