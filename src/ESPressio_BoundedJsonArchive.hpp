#pragma once
#include <charconv>
#include <cmath>
#include "ESPressio_BoundedDeserializer.hpp"

namespace ESPressio::Serializable {
namespace JsonDetail {
    template<class T> constexpr std::size_t Depth();
    template<class... T> constexpr std::size_t AlternativeDepth(std::tuple<T...>*) {
        std::size_t depth=0; ((depth=std::max(depth,Depth<T>())),...); return depth;
    }
    template<class T> constexpr std::size_t Depth() {
        using Traits=BoundedValueTraits<T>; constexpr auto k=Traits::Kind;
        if constexpr (k==SerializedValueKind::Object) {
            std::size_t depth=0;
            std::apply([&](const auto&... p) { ((depth=std::max(depth,Depth<typename std::decay_t<decltype(p)>::ValueType>())),...); },T::GetSerializableProperties());
            return depth+1;
        } else if constexpr (k==SerializedValueKind::Map) return 2+std::max(Depth<typename Traits::Key>(),Depth<typename Traits::Value>());
        else if constexpr (k==SerializedValueKind::Variant) return 1+AlternativeDepth(static_cast<typename Traits::Alternatives*>(nullptr));
        else if constexpr (k==SerializedValueKind::Optional) return Depth<typename Traits::Element>()+(BoundedValueTraits<typename Traits::Element>::Kind==SerializedValueKind::Optional ? 1 : 0);
        else if constexpr (k==SerializedValueKind::Sequence || k==SerializedValueKind::Set || k==SerializedValueKind::FixedArray) return 1+Depth<typename Traits::Element>();
        else return 0;
    }
    struct Writer {
        DirectBinaryDetail::BoundedOutput& Output;
        SerializationErrorCode Error=SerializationErrorCode::None;
        void Text(std::string_view value) noexcept {
            constexpr char hex[]="0123456789abcdef";
            Output.push_back('"');
            for (unsigned char c:value) {
                if (c=='"' || c=='\\') { Output.push_back('\\'); Output.push_back(c); }
                else if (c<0x20) {
                    for (auto x:std::array<char,4>{'\\','u','0','0'}) Output.push_back(x);
                    Output.push_back(hex[c>>4]); Output.push_back(hex[c&15]);
                } else Output.push_back(c);
            }
            Output.push_back('"');
        }
        void Name(std::string_view name) noexcept { Text(name); Output.push_back(':'); }
        template<class T> void Value(const T& value) {
            using Traits=BoundedValueTraits<T>; constexpr auto k=Traits::Kind;
            if constexpr (IsSerializable<T>) {
                Output.push_back('{'); Name("__schemaVersion"); Value(SerializationTraits<T>::CurrentVersion);
                std::apply([&](const auto&... p) { ((Output.push_back(','),Name(p.GetName()),Value(p.GetValue(value))),...); },BoundedDetail::SchemaStorage<T>::Tuple);
                Output.push_back('}');
            } else if constexpr (k==SerializedValueKind::String) Text(value.view());
            else if constexpr (k==SerializedValueKind::Optional) {
                if (value) {
                    if constexpr (BoundedValueTraits<typename T::value_type>::Kind==SerializedValueKind::Optional) { Output.push_back('{'); Name("value"); Value(*value); Output.push_back('}'); }
                    else Value(*value);
                } else for (char c:std::string_view("null")) Output.push_back(c);
            } else if constexpr (k==SerializedValueKind::Variant) {
                Output.push_back('{'); Name("index"); Value(static_cast<std::uint32_t>(value.index()));
                Output.push_back(','); Name("value"); std::visit([&](const auto& v) { Value(v); },value); Output.push_back('}');
            } else if constexpr (k==SerializedValueKind::Sequence || k==SerializedValueKind::FixedArray || k==SerializedValueKind::Set || k==SerializedValueKind::Map) {
                Output.push_back('['); std::size_t index=0;
                for (const auto& item:value) {
                    if (index++) Output.push_back(',');
                    if constexpr (k==SerializedValueKind::Map) {
                        Output.push_back('{'); Name("key"); Value(item.first); Output.push_back(','); Name("value"); Value(item.second); Output.push_back('}');
                    } else Value(item);
                }
                Output.push_back(']');
            } else if constexpr (std::is_enum_v<T>) {
                if constexpr (HasEnumSerializationMapping<T>) { if (auto* name=EnumToString(value)) { Text(name); return; } }
                Value(static_cast<std::underlying_type_t<T>>(value));
            } else if constexpr (std::is_same_v<T,bool>) {
                for (char c:std::string_view(value ? "true" : "false")) Output.push_back(c);
            } else if constexpr (std::is_arithmetic_v<T>) {
                if constexpr (std::is_floating_point_v<T>) if (!std::isfinite(value)) { Error=SerializationErrorCode::ValidationFailed; return; }
                std::array<char,64> bytes{};
                auto result=std::to_chars(bytes.data(),bytes.data()+bytes.size(),value);
                if (result.ec!=std::errc{}) { Error=SerializationErrorCode::NumericOutOfRange; return; }
                Output.insert(Output.end(),bytes.data(),result.ptr);
            } else static_assert(Detail::DependentFalse<T>::value,"Custom bounded value needs a JSON codec");
        }
    };
}
/// <summary>Serializes a bounded schema to compact canonical JSON in caller storage; floating point must be finite.</summary>
/// <remarks>Includes Sensitive data. Pretty-printing and presentation redaction are separate general archive operations.</remarks>
template<class T> BoundedSerializationResult SerializeBoundedJson(const T& object, std::uint8_t* data, std::size_t capacity) {
    static_assert(IsBoundedSerializable<T>,"Bounded JSON requires an explicit nonzero version and finite graph");
    (void)MaximumSerializedSize<T,JSON>;
    if (!data) return {SerializationErrorCode::ResourceLimitExceeded,0};
    DirectBinaryDetail::BoundedOutput output{data,capacity}; JsonDetail::Writer writer{output}; writer.Value(object);
    if (writer.Error!=SerializationErrorCode::None) return {writer.Error,0};
    if (!output.Valid) return {SerializationErrorCode::ResourceLimitExceeded,0};
    // Validation catches invalid UTF-8 supplied in textual fields, without allocating or publishing it.
    auto* cursor=static_cast<const std::uint8_t*>(data);
    if (!JsonDetail::SkipValue(cursor,data+output.Size,JsonDetail::Depth<T>()) || cursor!=data+output.Size) return {SerializationErrorCode::ValidationFailed,0};
    return {SerializationErrorCode::None,output.Size};
}
/// <summary>Decodes JSON directly under schema-derived byte, depth, string and collection bounds.</summary>
/// <remarks>Counts containers without copying, validates UTF-8/escapes, and checks decoded string capacity before copying.
/// Publication is transactional; old schema tree migrations and unknown properties are rejected.</remarks>
template<class T> BoundedSerializationResult DeserializeBoundedJson(const std::uint8_t* data, std::size_t size, T& object) {
    static_assert(IsBoundedSerializable<T>,"Bounded JSON requires an explicit nonzero version and finite graph");
    static_assert(std::is_nothrow_move_assignable_v<T>,"Bounded decode publication requires nonthrowing move assignment");
    if (size>MaximumSerializedSize<T,JSON>) return {SerializationErrorCode::ResourceLimitExceeded,0};
    if (!data || !size) return {SerializationErrorCode::MalformedInput,0};
    T candidate=object;
    DirectBinaryDetail::BoundedReader<JSON> reader{data,data+size,SerializationErrorCode::None,JsonDetail::Depth<T>()};
    if (!reader.Object(candidate)) return {reader.Error,0};
    JsonDetail::Space(reader.Cursor,reader.End);
    if (reader.Cursor!=reader.End) return {SerializationErrorCode::MalformedInput,0};
    object=std::move(candidate); return {SerializationErrorCode::None,size};
}
}
