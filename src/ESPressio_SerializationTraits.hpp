#pragma once

#include <type_traits>
#include <utility>

namespace ESPressio::Serializable {

    namespace Detail {

        template<typename T, typename = void>
        struct HasSerializableProperties : std::false_type {
        };

        template<typename T>
        struct HasSerializableProperties<
            T,
            std::void_t<decltype(T::GetSerializableProperties())>
        > : std::true_type {
        };

    }

    /// <summary>Indicates whether a type exposes the ESPressio serializable-property contract.</summary>
    /// <remarks>References and cv-qualifiers are removed before detecting <c>GetSerializableProperties()</c>.</remarks>
    template<typename T>
    inline constexpr bool IsSerializable =
        Detail::HasSerializableProperties<
            std::remove_cv_t<std::remove_reference_t<T>>
        >::value;

}
