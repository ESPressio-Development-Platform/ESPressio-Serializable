#pragma once
#include <array>
#include <cstdint>
#include <cstring>
#include <string_view>
#include "ESPressio_SerializationTraits.hpp"

#if defined(__has_builtin)
#  if __has_builtin(__builtin_bit_cast)
#    define ESPRESSIO_SERIALIZABLE_HAS_BUILTIN_BIT_CAST 1
#  endif
#endif
#ifndef ESPRESSIO_SERIALIZABLE_HAS_BUILTIN_BIT_CAST
#  define ESPRESSIO_SERIALIZABLE_HAS_BUILTIN_BIT_CAST 0
#endif

namespace ESPressio::Serializable {
struct StaticSchemaDescriptor;
/// <summary>Immutable shape of a value, including recursively reachable bounds and alternatives.</summary>
struct StaticValueDescriptor {
    SerializedValueKind Kind{};
    std::size_t Cardinality = 0;
    std::size_t DirectBinaryBytes = 0;
    std::size_t CborBytes = 0;
    std::size_t JsonBytes = 0;
    const StaticSchemaDescriptor* NestedSchema = nullptr;
    const StaticValueDescriptor* Element = nullptr;
    const StaticValueDescriptor* Mapped = nullptr;
    const StaticValueDescriptor* const* Alternatives = nullptr;
    std::size_t AlternativeCount = 0;
};
/// <summary>Read-only property metadata backed by one statically retained property tuple.</summary>
struct StaticPropertyDescriptor {
    std::string_view Name;
    SerializationPropertyFlags Flags{};
    std::size_t AliasCount = 0;
    const void* Property = nullptr;
    std::string_view (*AliasAt)(const void*, std::size_t) noexcept = nullptr;
    bool HasDefault = false;
    const StaticValueDescriptor* Value = nullptr;
    /// <summary>Returns an accepted alias, or an empty view when the index is out of range.</summary>
    std::string_view Alias(std::size_t index) const noexcept { return AliasAt ? AliasAt(Property, index) : std::string_view{}; }
};
/// <summary>Static schema view; no object ownership, allocation, mutable registry or application callbacks.</summary>
struct StaticSchemaDescriptor {
    std::uint32_t CurrentVersion = 0;
    std::uint32_t MinimumReadableVersion = 0;
    std::uint32_t MaximumReadableVersion = 0;
    std::size_t PropertyCount = 0;
    const StaticPropertyDescriptor* Properties = nullptr;
    std::uint64_t StructuralFingerprint = 0;
    std::size_t MaximumDirectBinaryBytes = 0;
    std::size_t MaximumCborBytes = 0;
    std::size_t MaximumJsonBytes = 0;
};

namespace BoundedDetail {
    template<class T> struct SchemaStorage;
    template<class T> struct ValueStorage;
    struct StructuralHash {
        std::uint64_t Value = 14695981039346656037ull;
        constexpr void Byte(std::uint8_t value) noexcept { Value = (Value ^ value) * 1099511628211ull; }
        constexpr void Integer(std::uint64_t value) noexcept {
            for (unsigned i = 0; i < 8; ++i) { Byte(static_cast<std::uint8_t>(value)); value >>= 8; }
        }
        constexpr void Text(std::string_view value) noexcept {
            Integer(value.size());
            for (char c : value) Byte(static_cast<std::uint8_t>(c));
        }
    };
    template<class T,class Sink> constexpr void HashShape(Sink& hash);
    template<class T,class Sink> constexpr void HashValue(Sink& hash, const T& value);
    template<class Sink,class... T> constexpr void HashAlternatives(Sink& hash, std::tuple<T...>*) {
        (HashShape<T>(hash), ...);
    }
    template<class... T> constexpr auto AlternativeDescriptors(std::tuple<T...>*) {
        return std::array<const StaticValueDescriptor*, sizeof...(T)>{&ValueStorage<T>::Value...};
    }
    template<class T> struct AlternativeStorage {
        inline static constexpr auto Values = AlternativeDescriptors(static_cast<typename BoundedValueTraits<T>::Alternatives*>(nullptr));
    };
    template<class Tuple, class Function, std::size_t... I>
    constexpr void VisitIndex(const Tuple& tuple, std::size_t index, Function function, std::index_sequence<I...>) {
        ((index == I ? (function(std::get<I>(tuple)), void()) : void()), ...);
    }
    template<class Tuple, class Function>
    constexpr void SortedProperties(const Tuple& tuple, Function function) {
        constexpr auto n = std::tuple_size<Tuple>::value;
        std::array<std::string_view, n> names{};
        std::array<std::size_t, n> order{};
        std::size_t i = 0;
        std::apply([&](const auto&... p) { ((names[i] = p.GetName(), order[i] = i, ++i), ...); }, tuple);
        for (std::size_t j = 1; j < n; ++j) {
            auto key = order[j]; auto k = j;
            while (k && names[order[k - 1]] > names[key]) { order[k] = order[k - 1]; --k; }
            order[k] = key;
        }
        for (auto index : order) VisitIndex(tuple, index, function, std::make_index_sequence<n>{});
    }
    template<class T>
    inline std::uint64_t FloatingBitsRuntime(const T& value) noexcept {
        static_assert(std::numeric_limits<T>::is_iec559 && (sizeof(T) == 4 || sizeof(T) == 8),
                      "Bounded floating metadata requires IEEE binary32 or binary64");
        if constexpr (sizeof(T) == 4) {
            std::uint32_t bits = 0;
            std::memcpy(&bits, &value, sizeof(bits));
            return bits;
        } else {
            std::uint64_t bits = 0;
            std::memcpy(&bits, &value, sizeof(bits));
            return bits;
        }
    }
    template<class T,class Sink> constexpr void HashValue(Sink& hash, const T& value) {
        using Traits = BoundedValueTraits<T>;
        constexpr auto kind = Traits::Kind;
        if constexpr (std::is_integral_v<T> || std::is_enum_v<T>) hash.Integer(static_cast<std::uint64_t>(value));
        else if constexpr (std::is_floating_point_v<T>) {
            // Hash the exact IEEE wire bits in canonical integer order. Signed zero and
            // NaN payloads remain distinct schema defaults/ranges. Newer compilers can
            // constant-evaluate the bit cast; older GCC retains the exact bits at static
            // descriptor initialization through memcpy without changing the semantics.
            static_assert(std::numeric_limits<T>::is_iec559 && (sizeof(T) == 4 || sizeof(T) == 8),
                          "Bounded floating metadata requires IEEE binary32 or binary64");
#if ESPRESSIO_SERIALIZABLE_HAS_BUILTIN_BIT_CAST
            if constexpr (sizeof(T) == 4) hash.Integer(__builtin_bit_cast(std::uint32_t, value));
            else hash.Integer(__builtin_bit_cast(std::uint64_t, value));
#else
            hash.Integer(FloatingBitsRuntime(value));
#endif
        } else if constexpr (kind == SerializedValueKind::String) hash.Text(value.view());
        else if constexpr (kind == SerializedValueKind::Object) {
            SortedProperties(T::GetSerializableProperties(), [&](const auto& p) { HashValue(hash, p.GetValue(value)); });
        } else if constexpr (kind == SerializedValueKind::Optional) {
            hash.Byte(value.has_value()); if (value) HashValue(hash, *value);
        } else if constexpr (kind == SerializedValueKind::Variant) {
            hash.Integer(value.index()); std::visit([&](const auto& item) { HashValue(hash, item); }, value);
        } else {
            hash.Integer(value.size());
            for (const auto& item : value) {
                if constexpr (kind == SerializedValueKind::Map) { HashValue(hash, item.first); HashValue(hash, item.second); }
                else HashValue(hash, item);
            }
        }
    }
    template<class T,class Sink> constexpr void HashShape(Sink& hash) {
        using Traits = BoundedValueTraits<T>;
        constexpr auto kind = Traits::Kind;
        hash.Byte(static_cast<std::uint8_t>(kind)); hash.Integer(Traits::Cardinality);
        if constexpr (std::is_arithmetic_v<T>) hash.Integer(std::numeric_limits<T>::digits);
        else if constexpr (std::is_enum_v<T>) {
            HashShape<std::underlying_type_t<T>>(hash);
            if constexpr (HasEnumSerializationMapping<T>) {
                constexpr auto entries = EnumSerializationTraits<T>::Values();
                std::array<std::size_t, entries.size()> order{};
                for (std::size_t i = 0; i < order.size(); ++i) order[i] = i;
                for (std::size_t i = 1; i < order.size(); ++i) {
                    auto key = order[i]; auto j = i;
                    while (j && std::string_view(entries[order[j-1]].Name) > entries[key].Name) { order[j] = order[j-1]; --j; }
                    order[j] = key;
                }
                hash.Integer(order.size());
                for (auto i : order) { hash.Text(entries[i].Name); HashValue(hash, entries[i].Value); }
            } else hash.Integer(0);
        } else if constexpr (kind == SerializedValueKind::Object) {
            hash.Integer(SerializationTraits<T>::CurrentVersion);
            hash.Integer(SerializationTraits<T>::MinimumReadableVersion);
            hash.Integer(SerializationTraits<T>::MaximumReadableVersion);
            SortedProperties(T::GetSerializableProperties(), [&](const auto& p) {
                using P = std::decay_t<decltype(p)>;
                hash.Text(p.GetName()); hash.Byte(static_cast<std::uint8_t>(p.GetFlags()));
                HashShape<typename P::ValueType>(hash);
                std::array<std::string_view, P::MaximumAliases> aliases{};
                for (std::size_t i = 0; i < p.GetAliasCount(); ++i) aliases[i] = p.GetAlias(i);
                for (std::size_t i = 1; i < p.GetAliasCount(); ++i) {
                    auto key = aliases[i]; auto j = i;
                    while (j && aliases[j-1] > key) { aliases[j] = aliases[j-1]; --j; }
                    aliases[j] = key;
                }
                hash.Integer(p.GetAliasCount());
                for (std::size_t i = 0; i < p.GetAliasCount(); ++i) hash.Text(aliases[i]);
                hash.Byte(p.HasDefault()); if (p.HasDefault()) HashValue(hash, p.GetDefault());
                hash.Byte(p.HasMinimum()); if (p.HasMinimum()) HashValue(hash, p.GetMinimum());
                hash.Byte(p.HasMaximum()); if (p.HasMaximum()) HashValue(hash, p.GetMaximum());
                // Opaque business-validator addresses are deliberately excluded.
            });
        } else if constexpr (kind == SerializedValueKind::Map) {
            HashShape<typename Traits::Key>(hash); HashShape<typename Traits::Value>(hash);
        } else if constexpr (kind == SerializedValueKind::Variant) {
            HashAlternatives(hash, static_cast<typename Traits::Alternatives*>(nullptr));
        } else if constexpr (kind != SerializedValueKind::String) HashShape<typename Traits::Element>(hash);
    }
    template<class T> struct ValueStorage {
        using Traits = BoundedValueTraits<T>;
        inline static constexpr StaticValueDescriptor Value = [] {
            StaticValueDescriptor v{Traits::Kind, Traits::Cardinality,
                Traits::template MaximumEncodedBytes<DirectBinary>, Traits::template MaximumEncodedBytes<CBOR>, Traits::template MaximumEncodedBytes<JSON>};
            constexpr auto k = Traits::Kind;
            if constexpr (k == SerializedValueKind::Object) v.NestedSchema = &SchemaStorage<T>::Value;
            else if constexpr (k == SerializedValueKind::Map) {
                v.Element = &ValueStorage<typename Traits::Key>::Value; v.Mapped = &ValueStorage<typename Traits::Value>::Value;
            } else if constexpr (k == SerializedValueKind::Variant) {
                v.Alternatives = AlternativeStorage<T>::Values.data(); v.AlternativeCount = AlternativeStorage<T>::Values.size();
            } else if constexpr (k == SerializedValueKind::Sequence || k == SerializedValueKind::FixedArray || k == SerializedValueKind::Set || k == SerializedValueKind::Optional)
                v.Element = &ValueStorage<typename Traits::Element>::Value;
            return v;
        }();
    };
    template<class T> struct SchemaStorage {
        static_assert(IsBoundedSerializable<T>, "Static Primitive schema requires an explicit version and bounded graph");
        inline static constexpr auto Tuple = T::GetSerializableProperties();
        inline static constexpr auto Properties = [] {
            std::array<StaticPropertyDescriptor, std::tuple_size<decltype(Tuple)>::value> values{};
            std::size_t i = 0;
            std::apply([&](const auto&... p) {
                auto add = [&](const auto& property) {
                    using P = std::decay_t<decltype(property)>;
                    values[i++] = {property.GetName(), property.GetFlags(), property.GetAliasCount(), &property,
                        [](const void* p, std::size_t n) noexcept -> std::string_view {
                            auto alias = static_cast<const P*>(p)->GetAlias(n); return alias ? alias : "";
                        }, property.HasDefault(), &ValueStorage<typename P::ValueType>::Value};
                };
                (add(p), ...);
            }, Tuple);
            return values;
        }();
#if ESPRESSIO_SERIALIZABLE_HAS_BUILTIN_BIT_CAST
        inline static constexpr StaticSchemaDescriptor Value = [] {
#else
        inline static const StaticSchemaDescriptor Value = [] {
#endif
            StructuralHash hash; hash.Text("ESPressio bounded schema v1;ESPB2;CBOR;JSON"); HashShape<T>(hash);
            return StaticSchemaDescriptor{SerializationTraits<T>::CurrentVersion, SerializationTraits<T>::MinimumReadableVersion,
                SerializationTraits<T>::MaximumReadableVersion, Properties.size(), Properties.data(), hash.Value,
                MaximumSerializedSize<T, DirectBinary>, MaximumSerializedSize<T, CBOR>, MaximumSerializedSize<T, JSON>};
        }();
    };
}
/// <summary>Returns static immutable schema metadata derived from the type's sole property tuple.</summary>
template<class T> constexpr const StaticSchemaDescriptor& SchemaDescriptor() noexcept { return BoundedDetail::SchemaStorage<T>::Value; }
/// <summary>Streams the complete canonical P3 schema semantics into a bounded family-owned hash sink.</summary>
/// <remarks>The sink provides noexcept Byte(uint8_t), Integer(uint64_t) and Text(string_view),
/// using eight-byte little-endian integers and length-prefixed text. It receives the same
/// domain/version/format, sorted properties/aliases, enum mappings, defaults and ranges as
/// StructuralFingerprint, without first reducing the input to that 64-bit digest. Families
/// add their own domain and contracts and produce their required full fingerprint.</remarks>
template<class T,class Sink> constexpr void WriteCanonicalSchema(Sink& sink) noexcept {
    static_assert(IsBoundedSerializable<T>, "Canonical schema requires bounded Serializable metadata");
    static_assert(noexcept(sink.Byte(std::uint8_t{})) && noexcept(sink.Integer(std::uint64_t{})) &&
                  noexcept(sink.Text(std::string_view{})), "Canonical schema sink must be noexcept");
    sink.Text("ESPressio bounded schema v1;ESPB2;CBOR;JSON");
    BoundedDetail::HashShape<T>(sink);
}

}

#undef ESPRESSIO_SERIALIZABLE_HAS_BUILTIN_BIT_CAST
