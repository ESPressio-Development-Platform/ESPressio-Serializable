#pragma once
#include <array>
#include <cstring>
#include <type_traits>

namespace ESPressio::Serializable {

    /// <summary>Associates one enumeration value with its serialized symbolic name.</summary>
    template<typename TEnum>
    struct EnumMappingEntry {
        TEnum Value;
        const char* Name;
    };

    /// <summary>Creates one enumeration serialization mapping entry.</summary>
    template<typename TEnum>
    constexpr EnumMappingEntry<TEnum> MakeEnumMapping(
        TEnum value,
        const char* name
    ) {
        return { value, name };
    }

    /// <summary>Customization point declaring symbolic serialization mappings for an enumeration type.</summary>
    template<typename TEnum>
    struct EnumSerializationTraits {
        static constexpr bool Enabled = false;
    };

    /// <summary>Indicates whether an enumeration has an enabled symbolic serialization mapping.</summary>
    template<typename TEnum>
    inline constexpr bool HasEnumSerializationMapping =
        EnumSerializationTraits<TEnum>::Enabled;

    /// <summary>Returns the mapped symbolic name for an enumeration value.</summary>
    /// <returns>The mapped name, or <c>nullptr</c> when no enabled mapping contains the value.</returns>
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

    /// <summary>Attempts to parse an enumeration value from its mapped symbolic name.</summary>
    /// <returns><c>true</c> when a matching enabled mapping was found.</returns>
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
