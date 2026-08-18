#pragma once
#include <type_traits>
namespace ESPressio::Serializable {
    class SerializationNode;
    template<typename T, typename = void>
    struct SerializationAdapter {
        static constexpr bool Supported = false;
    };
    template<typename T>
    inline constexpr bool HasSerializationAdapter =
        SerializationAdapter<
            std::remove_cv_t<std::remove_reference_t<T>>
        >::Supported;
}
