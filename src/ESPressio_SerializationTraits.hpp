#pragma once

#include <type_traits>
#include <utility>
#include "ESPressio_BoundedValueTraits.hpp"
#include "ESPressio_SerializationProperty.hpp"

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

namespace ESPressio::Serializable {
namespace BoundedDetail {
    template<class Tuple, std::size_t... I> constexpr std::size_t NameSlots(std::index_sequence<I...>) {
        return (std::size_t{0} + ... + (1 + std::decay_t<std::tuple_element_t<I,Tuple>>::MaximumAliases));
    }
    template<class T> constexpr bool ValidMetadata() {
        using Tuple = decltype(T::GetSerializableProperties());
        std::array<std::string_view, NameSlots<Tuple>(std::make_index_sequence<std::tuple_size<Tuple>::value>{})> names{};
        std::size_t count = 0;
        bool valid = true;
        auto add = [&](const char* name) {
            if (!name || !*name || Length(name) > 65535 || std::string_view(name)=="__schemaVersion" || !ValidUtf8(name)) { valid=false; return; }
            for (std::size_t i=0;i<count;++i) if (names[i]==name) { valid=false; return; }
            names[count++]=name;
        };
        std::apply([&](const auto&... p) {
            auto property = [&](const auto& item) {
                valid &= item.IsMetadataValid(); add(item.GetName());
                for (std::size_t i=0;i<item.GetAliasCount();++i) add(item.GetAlias(i));
            };
            (property(p),...);
        },T::GetSerializableProperties());
        return valid;
    }
    template<class T, class = void> struct ExplicitVersion : std::false_type {};
    template<class T> struct ExplicitVersion<T, std::void_t<decltype(T::GetSerializableSchemaVersion())>> : std::true_type {};
    template<class T> struct BoundedObject;
}
// Serializable object eligibility cannot be supplied as an arbitrary user byte count.
template<class T> struct BoundedValueTraits<T, std::enable_if_t<IsSerializable<T>>> {
    static constexpr bool IsBounded = BoundedDetail::BoundedObject<T>::IsBounded;
    static constexpr SerializedValueKind Kind = SerializedValueKind::Object;
    static constexpr std::size_t Cardinality = BoundedDetail::BoundedObject<T>::PropertyCount;
    template<class F> static constexpr std::size_t MaximumEncodedBytes = BoundedDetail::BoundedObject<T>::template ObjectBytes<F>();
};
namespace BoundedDetail {
    template<class Tuple, std::size_t... I>
    constexpr bool BoundedProperties(std::index_sequence<I...>) {
        return (IsBoundedWireValue<typename std::decay_t<std::tuple_element_t<I, Tuple>>::ValueType> && ...);
    }
    template<class T> struct BoundedObject {
        using Tuple = decltype(T::GetSerializableProperties());
        static constexpr std::size_t PropertyCount = std::tuple_size<Tuple>::value;
        static constexpr bool GraphBounded = BoundedProperties<Tuple>(std::make_index_sequence<PropertyCount>{});
        static constexpr bool IsBounded = [] {
            if constexpr (!ExplicitVersion<T>::value || !GraphBounded || HasSerializationAdapter<T>) return false;
            else return T::GetSerializableSchemaVersion() != 0 && PropertyCount < 65535 && ValidMetadata<T>();
        }();
        template<class F> static constexpr std::size_t ObjectBytes() {
            if constexpr (!IsBounded) return Overflow;
            else {
                std::size_t total = Add(ObjectOverhead<F>(), Add(NameBytes<F>(15), BoundedValueTraits<std::uint32_t>::template MaximumEncodedBytes<F>));
                std::apply([&](const auto&... property) {
                    auto add = [&](const auto& p) {
                        using V = typename std::decay_t<decltype(p)>::ValueType;
                        // Aliases can be longer than the canonical spelling on accepted ingress.
                        std::size_t name = Length(p.GetName());
                        for (std::size_t i = 0; i < p.GetAliasCount(); ++i) name = std::max(name, Length(p.GetAlias(i)));
                        if (name > 65535 || p.GetName() == nullptr) { total = Overflow; return; }
                        total = Add(total, Add(NameBytes<F>(name), BoundedValueTraits<V>::template MaximumEncodedBytes<F>));
                    };
                    (add(property), ...);
                }, T::GetSerializableProperties());
                return total;
            }
        }
    };
}
/// <summary>Normalized bounded object contract derived exclusively from GetSerializableProperties().</summary>
/// <remarks>Broad IsSerializable remains independent. Wire binding requires IsBounded and a supported finite format.
/// Current-only readable versions deliberately exclude the allocating general tree migration path.</remarks>
template<class T> struct SerializationTraits {
    using Type = std::remove_cv_t<std::remove_reference_t<T>>;
    static constexpr bool IsBounded = [] {
        if constexpr (!IsSerializable<Type>) return false;
        else return BoundedDetail::BoundedObject<Type>::IsBounded;
    }();
    static constexpr std::uint32_t CurrentVersion = [] {
        if constexpr (BoundedDetail::ExplicitVersion<Type>::value) return std::uint32_t(Type::GetSerializableSchemaVersion());
        else return std::uint32_t{0};
    }();
    static constexpr std::uint32_t MinimumReadableVersion = CurrentVersion;
    static constexpr std::uint32_t MaximumReadableVersion = CurrentVersion;
    template<class F> static constexpr std::size_t MaximumEncodedBytes = [] {
        static_assert(IsBounded, "Primitive serialization requires an explicit nonzero schema version and a bounded property graph");
        static_assert(BoundedDetail::KnownFormat<F>, "No bounded encoded-size proof for selected format");
        constexpr auto bytes = BoundedDetail::BoundedObject<Type>::template ObjectBytes<F>();
        constexpr auto complete = BoundedDetail::Add(bytes, std::is_same_v<F, DirectBinary> ? 5 : 0);
        static_assert(complete != BoundedDetail::Overflow, "Serialized property names or complete encoded size exceed the representable bound");
        return complete;
    }();
};
/// <summary>Stricter Primitive eligibility predicate, separate from general-purpose serialization.</summary>
template<class T> inline constexpr bool IsBoundedSerializable = SerializationTraits<T>::IsBounded;
/// <summary>Complete encoded object bound including format framing, version, names and full nested containers.</summary>
template<class T, class F> inline constexpr std::size_t MaximumSerializedSize = SerializationTraits<T>::template MaximumEncodedBytes<F>;
}
