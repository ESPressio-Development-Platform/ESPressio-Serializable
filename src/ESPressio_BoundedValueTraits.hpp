#pragma once
#include <algorithm>
#include <array>
#include <cstdint>
#include <limits>
#include <optional>
#include <tuple>
#include <type_traits>
#include <variant>
#include "ESPressio_BoundedContainers.hpp"
#include "ESPressio_EnumSerialization.hpp"
#include "ESPressio_SerializationAdapter.hpp"

namespace ESPressio::Serializable {
/// <summary>Format tags for complete conservative serialized-size derivation.</summary>
struct DirectBinary {};
struct CBOR {};
struct JSON {};
/// <summary>Portable schema value categories; never C++ layout or compiler type identities.</summary>
enum class SerializedValueKind : std::uint8_t {
    Unsupported, Boolean, SignedInteger, UnsignedInteger, Float32, Float64,
    Enumeration, String, Sequence, FixedArray, Map, Set, Optional, Variant, Object
};
template<class T> struct SerializationTraits;
namespace BoundedDetail {
    // Saturation makes overflow rejectable rather than underestimating a wire bound.
    inline constexpr std::size_t Overflow = std::numeric_limits<std::size_t>::max();
    constexpr std::size_t Add(std::size_t a, std::size_t b) noexcept {
        return a > Overflow - b ? Overflow : a + b;
    }
    constexpr std::size_t Multiply(std::size_t a, std::size_t b) noexcept {
        return b != 0 && a > Overflow / b ? Overflow : a * b;
    }
    constexpr std::size_t Length(const char* s) noexcept {
        if (!s) return 0;
        std::size_t n = 0;
        while (s[n]) ++n;
        return n;
    }
    constexpr bool ValidUtf8(std::string_view text) noexcept {
        for (std::size_t i=0;i<text.size();++i) {
            auto c=static_cast<std::uint8_t>(text[i]);
            if (c<0x80) continue;
            unsigned n=c>=0xc2 && c<=0xdf ? 2 : c>=0xe0 && c<=0xef ? 3 : c>=0xf0 && c<=0xf4 ? 4 : 0;
            if (!n || text.size()-i<n) return false;
            std::uint32_t point=c&((1u<<(7-n))-1);
            for (unsigned j=1;j<n;++j) {
                auto next=static_cast<std::uint8_t>(text[i+j]);
                if ((next&0xc0)!=0x80) return false;
                point=(point<<6)|(next&0x3f);
            }
            if ((n==3 && point<0x800) || (n==4 && point<0x10000) || point>0x10ffff || (point>=0xd800 && point<=0xdfff)) return false;
            i+=n-1;
        }
        return true;
    }
    template<class F> inline constexpr bool KnownFormat =
        std::is_same_v<F, DirectBinary> || std::is_same_v<F, CBOR> || std::is_same_v<F, JSON>;
    template<class F> constexpr std::size_t StringBytes(std::size_t n) noexcept {
        static_assert(KnownFormat<F>, "No bounded encoded-size proof for selected format");
        if constexpr (std::is_same_v<F, DirectBinary>) return Add(5, n);
        if constexpr (std::is_same_v<F, CBOR>) return Add(9, n);
        return Add(2, Multiply(6, n)); // Every byte may require a six-character JSON escape.
    }
    template<class F> constexpr std::size_t NameBytes(std::size_t n) noexcept {
        if constexpr (std::is_same_v<F, DirectBinary>) return Add(2, n);
        if constexpr (std::is_same_v<F, CBOR>) return StringBytes<F>(n);
        return Add(StringBytes<F>(n), 2); // colon and separating comma
    }
    template<class F> constexpr std::size_t ObjectOverhead() noexcept {
        if constexpr (std::is_same_v<F, DirectBinary>) return 3;
        if constexpr (std::is_same_v<F, CBOR>) return 9;
        return 2;
    }
    template<class F> constexpr std::size_t SequenceBytes(std::size_t n, std::size_t element) noexcept {
        if constexpr (std::is_same_v<F, DirectBinary>) return Add(5, Multiply(n, element));
        if constexpr (std::is_same_v<F, CBOR>) return Add(9, Multiply(n, element));
        return Add(2, Multiply(n, Add(element, 1)));
    }
}

/// <summary>Customization seam for bounded scalar/container graphs; object traits remain derived from properties.</summary>
/// <remarks>Custom specializations must provide a finite graph and a conservative bound for each supported format,
/// together with matching bounded codec/schema semantics. Unsupported types fail qualification.</remarks>
template<class T, class Enable = void>
struct BoundedValueTraits {
    static constexpr bool IsBounded = false;
    static constexpr SerializedValueKind Kind = SerializedValueKind::Unsupported;
    static constexpr std::size_t Cardinality = 0;
    template<class F> static constexpr std::size_t MaximumEncodedBytes = BoundedDetail::Overflow;
};

/// <summary>Wire proof excludes tree-only adapters, even when the underlying C++ shape is otherwise bounded.</summary>
template<class T> inline constexpr bool IsBoundedWireValue = BoundedValueTraits<T>::IsBounded && !HasSerializationAdapter<T>;

template<class T>
struct BoundedValueTraits<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
    static constexpr bool IsBounded = sizeof(T) <= 8 && !std::is_same_v<T, long double> &&
        (!std::is_floating_point_v<T> || (std::numeric_limits<T>::is_iec559 &&
         ((std::is_same_v<T, float> && sizeof(T) == 4) || (std::is_same_v<T, double> && sizeof(T) == 8))));
    static constexpr SerializedValueKind Kind = std::is_same_v<T, bool> ? SerializedValueKind::Boolean :
        std::is_same_v<T, float> ? SerializedValueKind::Float32 :
        std::is_same_v<T, double> ? SerializedValueKind::Float64 :
        std::is_signed_v<T> ? SerializedValueKind::SignedInteger : SerializedValueKind::UnsignedInteger;
    static constexpr std::size_t Cardinality = 1;
    template<class F> static constexpr std::size_t MaximumEncodedBytes = [] {
        static_assert(BoundedDetail::KnownFormat<F>, "No encoded-size proof for selected format");
        if constexpr (std::is_same_v<F, JSON>) return std::size_t{std::is_same_v<T, bool> ? 5u : 64u};
        if constexpr (std::is_same_v<F, CBOR>) return std::size_t{std::is_same_v<T, bool> ? 1u : 9u};
        return std::size_t{std::is_same_v<T, bool> ? 2u : std::is_same_v<T, float> ? 5u : 9u};
    }();
};
template<class T>
struct BoundedValueTraits<T, std::enable_if_t<std::is_enum_v<T>>> {
    using Underlying = std::underlying_type_t<T>;
    static constexpr bool IsBounded = IsBoundedWireValue<Underlying>;
    static constexpr SerializedValueKind Kind = SerializedValueKind::Enumeration;
    static constexpr std::size_t Cardinality = 1;
    template<class F> static constexpr std::size_t MaximumEncodedBytes = [] {
        auto maximum = BoundedValueTraits<Underlying>::template MaximumEncodedBytes<F>;
        if constexpr (HasEnumSerializationMapping<T>) {
            for (const auto& entry : EnumSerializationTraits<T>::Values())
                maximum = std::max(maximum, BoundedDetail::StringBytes<F>(BoundedDetail::Length(entry.Name)));
        }
        return maximum;
    }();
};
template<std::size_t N> struct BoundedValueTraits<BoundedString<N>> {
    static constexpr bool IsBounded = N <= std::numeric_limits<std::uint32_t>::max();
    static constexpr SerializedValueKind Kind = SerializedValueKind::String;
    static constexpr std::size_t Cardinality = N;
    template<class F> static constexpr std::size_t MaximumEncodedBytes = BoundedDetail::StringBytes<F>(N);
};
template<class T, std::size_t N> struct BoundedValueTraits<BoundedVector<T, N>> {
    using Element = T;
    static constexpr bool IsBounded = IsBoundedWireValue<T> && N <= std::numeric_limits<std::uint32_t>::max();
    static constexpr SerializedValueKind Kind = SerializedValueKind::Sequence;
    static constexpr std::size_t Cardinality = N;
    template<class F> static constexpr std::size_t MaximumEncodedBytes =
        BoundedDetail::SequenceBytes<F>(N, BoundedValueTraits<T>::template MaximumEncodedBytes<F>);
};
template<class T, std::size_t N> struct BoundedValueTraits<std::array<T, N>> : BoundedValueTraits<BoundedVector<T, N>> {
    static constexpr SerializedValueKind Kind = SerializedValueKind::FixedArray;
};
template<class T, std::size_t N> struct BoundedValueTraits<BoundedSet<T, N>> : BoundedValueTraits<BoundedVector<T, N>> {
    static constexpr SerializedValueKind Kind = SerializedValueKind::Set;
};
template<class K, class V, std::size_t N> struct BoundedValueTraits<BoundedMap<K, V, N>> {
    using Key = K;
    using Value = V;
    static constexpr bool IsBounded = IsBoundedWireValue<K> && IsBoundedWireValue<V> && N <= std::numeric_limits<std::uint32_t>::max();
    static constexpr SerializedValueKind Kind = SerializedValueKind::Map;
    static constexpr std::size_t Cardinality = N;
    template<class F> static constexpr std::size_t MaximumEncodedBytes = [] {
        using namespace BoundedDetail;
        auto entry = Add(ObjectOverhead<F>(), Add(NameBytes<F>(3), NameBytes<F>(5)));
        entry = Add(entry, BoundedValueTraits<K>::template MaximumEncodedBytes<F>);
        entry = Add(entry, BoundedValueTraits<V>::template MaximumEncodedBytes<F>);
        return SequenceBytes<F>(N, entry);
    }();
};
template<class T> struct BoundedValueTraits<std::optional<T>> {
    using Element = T;
    static constexpr bool IsBounded = IsBoundedWireValue<T>;
    static constexpr SerializedValueKind Kind = SerializedValueKind::Optional;
    static constexpr std::size_t Cardinality = 1;
    template<class F> static constexpr std::size_t MaximumEncodedBytes = [] {
        auto present=BoundedValueTraits<T>::template MaximumEncodedBytes<F>;
        // A present outer optional containing an absent inner optional is distinct from an absent outer value.
        if constexpr (BoundedValueTraits<T>::Kind==SerializedValueKind::Optional)
            present=BoundedDetail::Add(present,BoundedDetail::Add(BoundedDetail::ObjectOverhead<F>(),BoundedDetail::NameBytes<F>(5)));
        return std::max(std::is_same_v<F,JSON> ? std::size_t{4} : std::size_t{1},present);
    }();
};
template<class... T> struct BoundedValueTraits<std::variant<T...>> {
    using Alternatives = std::tuple<T...>;
    static constexpr bool IsBounded = (IsBoundedWireValue<T> && ...);
    static constexpr SerializedValueKind Kind = SerializedValueKind::Variant;
    static constexpr std::size_t Cardinality = sizeof...(T);
    // Variant is an object with an unsigned index and one value, not a native-layout union.
    template<class F> static constexpr std::size_t MaximumEncodedBytes = [] {
        using namespace BoundedDetail;
        auto maximum = std::max({BoundedValueTraits<T>::template MaximumEncodedBytes<F>...});
        return Add(Add(ObjectOverhead<F>(), Add(NameBytes<F>(5), NameBytes<F>(5))),
            Add(BoundedValueTraits<std::uint32_t>::template MaximumEncodedBytes<F>, maximum));
    }();
};
}
