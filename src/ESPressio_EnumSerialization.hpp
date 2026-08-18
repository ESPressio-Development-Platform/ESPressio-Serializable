#pragma once
#include <array>
#include <cstring>
#include <type_traits>

namespace ESPressio::Serializable {

    template<typename TEnum>
    struct EnumMappingEntry {
        TEnum Value;
        const char* Name;
    };

    template<typename TEnum>
    constexpr EnumMappingEntry<TEnum> MakeEnumMapping(
        TEnum value,
        const char* name
    ) {
        return { value, name };
    }

    template<typename TEnum>
    struct EnumSerializationTraits {
        static constexpr bool Enabled = false;
    };

    template<typename TEnum>
    inline constexpr bool HasEnumSerializationMapping =
        EnumSerializationTraits<TEnum>::Enabled;

    template<typename TEnum>
    const char* EnumToString(TEnum value) {
        if constexpr (!HasEnumSerializationMapping<TEnum>) {
            return nullptr;
        } else {
            for (const auto& entry : EnumSerializationTraits<TEnum>::Values()) {
                if (entry.Value == value) {
                    return entry.Name;
                }
            }
            return nullptr;
        }
    }

    template<typename TEnum>
    bool EnumFromString(const char* name, TEnum& value) {
        if constexpr (!HasEnumSerializationMapping<TEnum>) {
            return false;
        } else {
            if (name == nullptr) {
                return false;
            }
            for (const auto& entry : EnumSerializationTraits<TEnum>::Values()) {
                if (std::strcmp(entry.Name, name) == 0) {
                    value = entry.Value;
                    return true;
                }
            }
            return false;
        }
    }
}

#define ESPRESSIO_ENUM_VALUE(Value, Name) \
    ::ESPressio::Serializable::MakeEnumMapping(Value, Name)

#define ESPRESSIO_ENUM_MAPPING(EnumType, ...) \
    namespace ESPressio::Serializable { \
        template<> \
        struct EnumSerializationTraits<EnumType> { \
            static constexpr bool Enabled = true; \
            static constexpr auto Values() { \
                return std::array{ __VA_ARGS__ }; \
            } \
        }; \
    }
