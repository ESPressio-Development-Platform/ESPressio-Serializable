#pragma once
#include <array>
#include <charconv>
#include "ESPressio_BoundedJsonTokens.hpp"
#include <cstring>
#include <string_view>
#include "ESPressio_DirectBinaryArchive.hpp"
#include "ESPressio_SchemaDescriptor.hpp"

namespace ESPressio::Serializable {
/// <summary>Allocation-free result for bounded archive operations. Failure never publishes a partially decoded object.</summary>
struct BoundedSerializationResult {
    SerializationErrorCode Error = SerializationErrorCode::None;
    std::size_t Bytes = 0;
    constexpr explicit operator bool() const noexcept { return Error == SerializationErrorCode::None; }
};
namespace DirectBinaryDetail {
    /// <summary>Caller-owned output used by the same ESPB encoder as general DirectBinary archives.</summary>
    struct BoundedOutput {
        std::uint8_t* Data;
        std::size_t Capacity;
        std::size_t Size = 0;
        bool Valid = true;
        SerializationErrorCode Error = SerializationErrorCode::None;
        void push_back(std::uint8_t byte) noexcept {
            if (Size == Capacity) { Valid = false; return; }
            Data[Size++] = byte;
        }
        std::uint8_t* end() noexcept { return Data ? Data + Size : nullptr; }
        template<class Iterator> void insert(std::uint8_t*, Iterator first, Iterator last) noexcept {
            for (; first != last; ++first) push_back(static_cast<std::uint8_t>(*first));
        }
    };
    /// <summary>Direct bounded cursor. Object schemas drive traversal; unknown or duplicate fields never recurse.</summary>
    template<class Format = DirectBinary>
    struct BoundedReader {
        const std::uint8_t* Cursor;
        const std::uint8_t* End;
        SerializationErrorCode Error = SerializationErrorCode::None;
        std::size_t MaximumDepth = 0;
        bool Fail(SerializationErrorCode error = SerializationErrorCode::MalformedInput) noexcept {
            if (Error == SerializationErrorCode::None) Error = error;
            return false;
        }
        bool Tag(SerializationNodeType type) noexcept {
            return Cursor < End && *Cursor++ == static_cast<std::uint8_t>(type) ? true : Fail();
        }
        bool NameEquals(std::string_view actual, std::string_view expected) const noexcept {
            if constexpr (std::is_same_v<Format,JSON>) return JsonDetail::Equal(actual,expected);
            else return actual==expected;
        }
        bool Separator() noexcept {
            if constexpr (std::is_same_v<Format,JSON>) {
                JsonDetail::Space(Cursor,End); return Cursor<End && *Cursor++==',' ? true : Fail();
            } else return true;
        }
        bool Close(bool object) noexcept {
            if constexpr (std::is_same_v<Format,JSON>) {
                JsonDetail::Space(Cursor,End); return Cursor<End && *Cursor++==(object ? '}' : ']') ? true : Fail();
            } else return true;
        }
        bool CborArgument(std::uint8_t major, std::uint64_t& argument) noexcept {
            if (Cursor == End || (*Cursor >> 5) != major) return Fail();
            auto info = *Cursor++ & 31;
            if (info < 24) { argument=info; return true; }
            if (info > 27) return Fail(); // No indefinite-length allocation or unbounded scan.
            unsigned bytes = 1u << (info-24);
            if (std::size_t(End-Cursor) < bytes) return Fail();
            argument=0;
            while (bytes--) argument=(argument<<8)|*Cursor++;
            return true;
        }
        bool Count(bool object, std::uint32_t& count) noexcept {
            if constexpr (std::is_same_v<Format,JSON>) {
                auto* scan=Cursor;
                if (!JsonDetail::Container(scan,End,MaximumDepth,object,count)) return Fail();
                JsonDetail::Space(Cursor,End); ++Cursor; return true;
            } else if constexpr (std::is_same_v<Format,DirectBinary>) {
                if (!Tag(object ? SerializationNodeType::Object : SerializationNodeType::Array)) return false;
                if (object) { std::uint16_t n{}; if (!ReadU16(Cursor,End,n)) return Fail(); count=n; return true; }
                return ReadU32(Cursor,End,count) ? true : Fail();
            } else {
                std::uint64_t n{};
                if (!CborArgument(object ? 5 : 4,n)) return false;
                if (n > std::numeric_limits<std::uint32_t>::max()) return Fail(SerializationErrorCode::ResourceLimitExceeded);
                count=static_cast<std::uint32_t>(n); return true;
            }
        }
        bool IsString() const noexcept {
            if constexpr (std::is_same_v<Format,JSON>) return Cursor<End && *Cursor=='"';
            else if constexpr (std::is_same_v<Format,DirectBinary>) return Cursor<End && *Cursor==static_cast<std::uint8_t>(SerializationNodeType::String);
            else return Cursor<End && (*Cursor>>5)==3;
        }
        bool Null() noexcept {
            if constexpr (std::is_same_v<Format,JSON>) { JsonDetail::Space(Cursor,End); return JsonDetail::Literal(Cursor,End,"null"); }
            constexpr std::uint8_t code = std::is_same_v<Format,DirectBinary> ? static_cast<std::uint8_t>(SerializationNodeType::Null) : 0xf6;
            if (Cursor<End && *Cursor==code) { ++Cursor; return true; } return false;
        }
        bool TextLength(std::uint32_t& size, bool name=false) noexcept {
            if constexpr (std::is_same_v<Format,DirectBinary>) {
                if (name) { std::uint16_t n{}; if (!ReadU16(Cursor,End,n)) return Fail(); size=n; return true; }
                return Tag(SerializationNodeType::String) && ReadU32(Cursor,End,size) ? true : Fail();
            } else {
                std::uint64_t n{};
                if (!CborArgument(3,n)) return false;
                if (n > std::numeric_limits<std::uint32_t>::max()) return Fail(SerializationErrorCode::ResourceLimitExceeded);
                size=static_cast<std::uint32_t>(n); return true;
            }
        }
        bool Name(std::string_view& name) noexcept {
            if constexpr (std::is_same_v<Format,JSON>) {
                if (!JsonDetail::String(Cursor,End,name)) return Fail();
                JsonDetail::Space(Cursor,End); return Cursor<End && *Cursor++==':' ? true : Fail();
            }
            std::uint32_t size{};
            if (!TextLength(size,true) || std::size_t(End-Cursor) < size) return Fail();
            name = {reinterpret_cast<const char*>(Cursor),size}; Cursor += size; return true;
        }
        template<class T> bool Number(T& value) noexcept {
            if constexpr (std::is_same_v<Format,JSON>) {
                JsonDetail::Space(Cursor,End);
                if constexpr (std::is_same_v<T,bool>) {
                    if (JsonDetail::Literal(Cursor,End,"true")) { value=true; return true; }
                    if (JsonDetail::Literal(Cursor,End,"false")) { value=false; return true; }
                    return Fail();
                } else {
                    std::string_view number;
                    if (!JsonDetail::Number(Cursor,End,number)) return Fail();
                    auto result=std::from_chars(number.data(),number.data()+number.size(),value);
                    if (result.ec!=std::errc{} || result.ptr!=number.data()+number.size()) return Fail(SerializationErrorCode::NumericOutOfRange);
                    if constexpr (std::is_floating_point_v<T>) if (!std::isfinite(value)) return Fail(SerializationErrorCode::NumericOutOfRange);
                    return true;
                }
            } else if constexpr (std::is_same_v<Format,DirectBinary>) {
                if constexpr (std::is_same_v<T,bool>) {
                    if (!Tag(SerializationNodeType::Boolean) || Cursor==End || *Cursor>1) return Fail();
                    value=*Cursor++!=0; return true;
                } else {
                    if constexpr (std::is_same_v<T,float>) if (*Cursor!=static_cast<std::uint8_t>(SerializationNodeType::Float32)) return Fail();
                    return DecodeValue(Cursor,End,value,DeserializationOptions{}) ? true : Fail(SerializationErrorCode::NumericOutOfRange);
                }
            } else {
                if constexpr (std::is_same_v<T,bool>) {
                    if (*Cursor!=0xf4 && *Cursor!=0xf5) return Fail();
                    value=*Cursor++==0xf5; return true;
                } else if constexpr (std::is_integral_v<T>) {
                    auto major=*Cursor>>5; std::uint64_t raw{};
                    if (major>1 || !CborArgument(major,raw)) return Fail();
                    if (major==0) {
                        if (raw>static_cast<std::uint64_t>(std::numeric_limits<T>::max())) return Fail(SerializationErrorCode::NumericOutOfRange);
                        value=static_cast<T>(raw);
                    } else {
                        if constexpr (!std::is_signed_v<T>) return Fail(SerializationErrorCode::NumericOutOfRange);
                        else {
                            if (raw>static_cast<std::uint64_t>(std::numeric_limits<T>::max())) return Fail(SerializationErrorCode::NumericOutOfRange);
                            value=static_cast<T>(-1-static_cast<std::int64_t>(raw));
                        }
                    }
                    return true;
                } else {
                    auto tag=*Cursor++; unsigned bytes=tag==0xfa ? 4 : tag==0xfb ? 8 : 0;
                    if (!bytes || std::size_t(End-Cursor)<bytes) return Fail();
                    std::uint64_t raw{}; for (unsigned i=0;i<bytes;++i) raw=(raw<<8)|*Cursor++;
                    double decoded{};
                    if (bytes==4) { auto bits=static_cast<std::uint32_t>(raw); float number{}; std::memcpy(&number,&bits,4); decoded=number; }
                    else std::memcpy(&decoded,&raw,8);
                    if constexpr (std::is_same_v<T,float>) {
                        if (std::isfinite(decoded) && (decoded>std::numeric_limits<float>::max() || decoded < -std::numeric_limits<float>::max())) return Fail(SerializationErrorCode::NumericOutOfRange);
                    }
                    value=static_cast<T>(decoded); return true;
                }
            }
        }
        template<class T> bool Value(T& value) {
            using Traits = BoundedValueTraits<T>;
            static_assert(Traits::IsBounded, "Bounded decode encountered an unbounded value type");
            constexpr auto kind = Traits::Kind;
            if constexpr (std::is_same_v<Format,JSON>) JsonDetail::Space(Cursor,End);
            if (Cursor == End) return Fail();
            if constexpr (IsSerializable<T>) return Object(value);
            else if constexpr (kind == SerializedValueKind::String) {
                if constexpr (std::is_same_v<Format,JSON>) {
                    std::string_view raw;
                    if (!JsonDetail::String(Cursor,End,raw)) return Fail();
                    std::size_t length=0;
                    if (!JsonDetail::DecodeString(raw,[&](std::uint8_t) noexcept { return ++length<=T::capacity(); })) return Fail(SerializationErrorCode::ResourceLimitExceeded);
                    value.clear();
                    return JsonDetail::DecodeString(raw,[&](std::uint8_t c) noexcept { return value.push_back(static_cast<char>(c)); });
                }
                std::uint32_t size{};
                if (!TextLength(size)) return false;
                if (size > T::capacity()) return Fail(SerializationErrorCode::ResourceLimitExceeded);
                if (size > std::size_t(End-Cursor)) return Fail();
                if constexpr (std::is_same_v<Format,CBOR>)
                    if (!BoundedDetail::ValidUtf8({reinterpret_cast<const char*>(Cursor),size})) return Fail();
                value.assign({reinterpret_cast<const char*>(Cursor),size}); Cursor += size; return true;
            } else if constexpr (kind == SerializedValueKind::Optional) {
                if (Null()) { value.reset(); return true; }
                typename T::value_type item{};
                if constexpr (BoundedValueTraits<typename T::value_type>::Kind==SerializedValueKind::Optional) {
                    std::uint32_t count{}; std::string_view name;
                    if (!Count(true,count) || count!=1 || !Name(name) || !NameEquals(name,"value") || !Value(item) || !Close(true)) return Fail();
                } else if (!Value(item)) return false;
                value = std::move(item); return true;
            } else if constexpr (kind == SerializedValueKind::Variant) {
                std::uint32_t count{}; std::string_view name;
                std::uint32_t index{};
                // Canonical variant objects carry the discriminator before the selected alternative.
                if (!Count(true,count) || count != 2 ||
                    !Name(name) || !NameEquals(name,"index") || !Value(index) || !Separator() || !Name(name) || !NameEquals(name,"value")) return Fail();
                if (!Detail::DecodeVariantAlternative(index,value,[&](auto& item) { return Value(item); })) return Fail();
                return Close(true);
            } else if constexpr (kind == SerializedValueKind::Sequence || kind == SerializedValueKind::Set || kind == SerializedValueKind::FixedArray || kind == SerializedValueKind::Map) {
                std::uint32_t count{};
                if (!Count(false,count)) return Fail();
                if (count > Traits::Cardinality) return Fail(SerializationErrorCode::ResourceLimitExceeded);
                if constexpr (kind == SerializedValueKind::FixedArray) {
                    if (count != Traits::Cardinality) return Fail();
                    std::size_t index=0;
                    for (auto& item : value) { if (index++ && !Separator()) return false; if (!Value(item)) return false; }
                } else {
                    value.clear();
                    for (std::uint32_t i = 0; i < count; ++i) {
                        if (i && !Separator()) return false;
                        if constexpr (kind == SerializedValueKind::Map) {
                            std::uint32_t members{}; std::string_view name;
                            typename T::key_type key{}; typename T::mapped_type mapped{};
                            if (!Count(true,members) || members != 2 ||
                                !Name(name) || !NameEquals(name,"key") || !Value(key) || !Separator() || !Name(name) || !NameEquals(name,"value") || !Value(mapped) || !Close(true)) return Fail();
                            if (!value.insert(key,mapped)) return Fail(SerializationErrorCode::DuplicateValue);
                        } else {
                            typename T::value_type item{};
                            if (!Value(item)) return false;
                            if constexpr (kind == SerializedValueKind::Set) {
                                if (!value.insert(item)) return Fail(SerializationErrorCode::DuplicateValue);
                            } else if (!value.push_back(std::move(item))) return Fail(SerializationErrorCode::ResourceLimitExceeded);
                        }
                    }
                }
                return Close(false);
            } else if constexpr (std::is_enum_v<T>) {
                if constexpr (HasEnumSerializationMapping<T>) {
                    if (IsString()) {
                        if constexpr (std::is_same_v<Format,JSON>) {
                            std::string_view raw;
                            if (!JsonDetail::String(Cursor,End,raw)) return Fail();
                            for (const auto& entry : EnumSerializationTraits<T>::Values()) if (NameEquals(raw,entry.Name)) { value=entry.Value; return true; }
                            return Fail(SerializationErrorCode::UnknownEnumValue);
                        }
                        std::uint32_t size{};
                        if (!TextLength(size) || size > std::size_t(End-Cursor)) return Fail();
                        std::string_view name{reinterpret_cast<const char*>(Cursor),size};
                        for (const auto& entry : EnumSerializationTraits<T>::Values()) if (name == entry.Name) {
                            value = entry.Value; Cursor += size; return true;
                        }
                        return Fail(SerializationErrorCode::UnknownEnumValue);
                    }
                }
                std::underlying_type_t<T> raw{};
                if (!Value(raw)) return false;
                value = static_cast<T>(raw); return true;
            } else if constexpr (std::is_arithmetic_v<T>) {
                return Number(value);
            } else {
                static_assert(Detail::DependentFalse<T>::value, "Custom bounded value requires a matching allocation-free codec");
            }
        }
        template<class T> bool Object(T& object) {
            const auto& properties = BoundedDetail::SchemaStorage<T>::Tuple;
            constexpr auto n = std::tuple_size<std::decay_t<decltype(properties)>>::value;
            std::array<bool,n> seen{};
            bool versionSeen = false;
            std::uint32_t count{};
            if (!Count(true,count)) return Fail();
            if (count > n + 1) return Fail(SerializationErrorCode::ResourceLimitExceeded);
            for (std::uint32_t entry = 0; entry < count; ++entry) {
                if (entry && !Separator()) return false;
                std::string_view name;
                if (!Name(name)) return false;
                if (NameEquals(name,"__schemaVersion")) {
                    std::uint32_t version{};
                    if (versionSeen) return Fail(SerializationErrorCode::DuplicateValue);
                    versionSeen = true;
                    if (!Value(version)) return false;
                    if (version != SerializationTraits<T>::CurrentVersion) return Fail(SerializationErrorCode::UnsupportedSchemaVersion);
                    continue;
                }
                bool found = false; bool valid = true; std::size_t index = 0;
                std::apply([&](const auto&... p) {
                    auto read = [&](const auto& property) {
                        auto i = index++; bool matches = NameEquals(name,property.GetName());
                        for (std::size_t a = 0; a < property.GetAliasCount(); ++a) matches |= NameEquals(name,property.GetAlias(a));
                        if (!matches) return;
                        if (found || seen[i]) { valid = Fail(SerializationErrorCode::DuplicateValue); found = true; return; }
                        found = true; seen[i] = true;
                        using V = typename std::decay_t<decltype(property)>::ValueType;
                        V candidate{};
                        if (!Value(candidate)) { valid = false; return; }
                        if (!property.ValidateValue(candidate)) { valid = Fail(SerializationErrorCode::ValidationFailed); return; }
                        // Canonical wire reconstruction hydrates semantic data, including fields
                        // that are read-only to presentation/tooling mutation. General archives
                        // retain their existing ReadOnly mutation policy.
                        property.GetValue(object) = std::move(candidate);
                    };
                    (read(p), ...);
                }, properties);
                if (!found || !valid) return Fail();
            }
            if (!Close(true)) return false;
            if (!versionSeen) return Fail(SerializationErrorCode::UnsupportedSchemaVersion);
            bool valid = true; std::size_t index = 0;
            std::apply([&](const auto&... p) {
                auto finish = [&](const auto& property) {
                    auto i = index++;
                    if (seen[i]) return;
                    if (property.HasDefault()) {
                        if (!property.ValidateValue(property.GetDefault())) { valid = Fail(SerializationErrorCode::ValidationFailed); return; }
                        property.GetValue(object) = property.GetDefault();
                    } else if (property.IsRequired()) valid = Fail(SerializationErrorCode::MissingRequiredProperty);
                };
                (finish(p), ...);
            }, properties);
            return valid;
        }
    };
}
/// <summary>Encodes a bounded object into caller storage using the canonical ESPB v2 representation.</summary>
/// <remarks>Includes Sensitive properties; no presentation redaction policy is accepted. Output bytes on failure are unpublished scratch.</remarks>
template<class T> BoundedSerializationResult SerializeDirectBinary(const T& object, std::uint8_t* data, std::size_t capacity) {
    static_assert(IsBoundedSerializable<T>, "Bounded wire encode requires an explicit schema version and bounded property graph");
    (void)MaximumSerializedSize<T,DirectBinary>;
    if (!data) return {SerializationErrorCode::ResourceLimitExceeded,0};
    DirectBinaryDetail::BoundedOutput output{data,capacity};
    for (auto byte : std::array<std::uint8_t,5>{'E','S','P','B',2}) output.push_back(byte);
    DirectBinaryDetail::EncodeObject(output,object);
    return {output.Valid ? SerializationErrorCode::None : SerializationErrorCode::ResourceLimitExceeded, output.Valid ? output.Size : 0};
}
/// <summary>Decodes current-version ESPB directly into bounded temporary storage and publishes only complete success.</summary>
/// <remarks>Rejects complete oversize before traversal. Scratch is one T plus fixed schema-index bits and one value per nesting level.
/// Unknown fields, duplicate aliases, unsupported older versions and over-bound nested values fail without a heap tree.</remarks>
template<class T> BoundedSerializationResult DeserializeBoundedDirectBinary(const std::uint8_t* data, std::size_t size, T& object) {
    static_assert(IsBoundedSerializable<T>, "Bounded wire decode requires an explicit schema version and bounded property graph");
    static_assert(std::is_nothrow_move_assignable_v<T>, "Bounded decode publication requires nonthrowing move assignment");
    if (size > MaximumSerializedSize<T,DirectBinary>) return {SerializationErrorCode::ResourceLimitExceeded,0};
    if (!data || size < 8 || data[0]!='E' || data[1]!='S' || data[2]!='P' || data[3]!='B' || data[4]!=2)
        return {SerializationErrorCode::MalformedInput,0};
    T candidate = object;
    DirectBinaryDetail::BoundedReader<> reader{data+5,data+size};
    if (!reader.Object(candidate)) return {reader.Error,0};
    if (reader.Cursor != reader.End) return {SerializationErrorCode::MalformedInput,0};
    object = std::move(candidate); return {SerializationErrorCode::None,size};
}
}
