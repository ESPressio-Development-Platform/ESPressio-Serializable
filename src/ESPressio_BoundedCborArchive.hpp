#pragma once
#include "ESPressio_BoundedDeserializer.hpp"
#include "ESPressio_CborEncoding.hpp"

namespace ESPressio::Serializable {
namespace CborDetail {
    template<class Output, class T> void Encode(Output& output, const T& value) {
        using Traits=BoundedValueTraits<T>;
        static_assert(Traits::IsBounded, "Bounded CBOR requires a finite value graph");
        constexpr auto kind=Traits::Kind;
        if constexpr (IsSerializable<T>) {
            const auto& properties=BoundedDetail::SchemaStorage<T>::Tuple;
            Argument(output,5,std::tuple_size<std::decay_t<decltype(properties)>>::value+1);
            Text(output,"__schemaVersion"); Encode(output,SerializationTraits<T>::CurrentVersion);
            std::apply([&](const auto&... p) { ((Text(output,p.GetName()),Encode(output,p.GetValue(value))),...); },properties);
        } else if constexpr (kind==SerializedValueKind::String) {
            if (!BoundedDetail::ValidUtf8(value.view())) { output.Error=SerializationErrorCode::ValidationFailed; return; }
            Text(output,value.view());
        }
        else if constexpr (kind==SerializedValueKind::Optional) {
            if (!value) output.push_back(0xf6);
            else {
                if constexpr (BoundedValueTraits<typename T::value_type>::Kind==SerializedValueKind::Optional) { Argument(output,5,1); Text(output,"value"); }
                Encode(output,*value);
            }
        } else if constexpr (kind==SerializedValueKind::Variant) {
            Argument(output,5,2); Text(output,"index"); Encode(output,static_cast<std::uint32_t>(value.index()));
            Text(output,"value"); std::visit([&](const auto& item) { Encode(output,item); },value);
        } else if constexpr (kind==SerializedValueKind::Sequence || kind==SerializedValueKind::FixedArray || kind==SerializedValueKind::Set || kind==SerializedValueKind::Map) {
            Argument(output,4,value.size());
            for (const auto& item:value) {
                if constexpr (kind==SerializedValueKind::Map) {
                    Argument(output,5,2); Text(output,"key"); Encode(output,item.first); Text(output,"value"); Encode(output,item.second);
                } else Encode(output,item);
            }
        } else if constexpr (std::is_enum_v<T>) {
            if constexpr (HasEnumSerializationMapping<T>) {
                if (auto* name=EnumToString(value)) { Text(output,name); return; }
            }
            Encode(output,static_cast<std::underlying_type_t<T>>(value));
        } else if constexpr (std::is_same_v<T,bool>) output.push_back(value ? 0xf5 : 0xf4);
        else if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_signed_v<T>) {
                if (value<0) { Argument(output,1,static_cast<std::uint64_t>(-1-static_cast<std::int64_t>(value))); return; }
            }
            Argument(output,0,static_cast<std::uint64_t>(value));
        } else if constexpr (std::is_floating_point_v<T>) {
            static_assert(sizeof(T)==4 || sizeof(T)==8, "Unsupported CBOR floating point representation");
            using Bits=std::conditional_t<sizeof(T)==4,std::uint32_t,std::uint64_t>;
            Bits bits{}; std::memcpy(&bits,&value,sizeof(T));
            output.push_back(sizeof(T)==4 ? 0xfa : 0xfb);
            for (unsigned i=sizeof(T);i!=0;--i) output.push_back(static_cast<std::uint8_t>(bits>>((i-1)*8)));
        } else static_assert(Detail::DependentFalse<T>::value,"Custom bounded value needs a matching CBOR codec");
    }
}
/// <summary>Serializes canonical definite-length CBOR directly into caller storage, including Sensitive fields.</summary>
/// <remarks>The property graph, enum mappings and scalar representation are shared with CborArchive; no node tree is built.</remarks>
template<class T> BoundedSerializationResult SerializeBoundedCbor(const T& object, std::uint8_t* data, std::size_t capacity) {
    static_assert(IsBoundedSerializable<T>,"Bounded CBOR requires an explicit nonzero version and finite graph");
    (void)MaximumSerializedSize<T,CBOR>;
    if (!data) return {SerializationErrorCode::ResourceLimitExceeded,0};
    DirectBinaryDetail::BoundedOutput output{data,capacity}; CborDetail::Encode(output,object);
    if (output.Error!=SerializationErrorCode::None) return {output.Error,0};
    return {output.Valid ? SerializationErrorCode::None : SerializationErrorCode::ResourceLimitExceeded,output.Valid ? output.Size : 0};
}
/// <summary>Decodes current-version CBOR with per-field capacity checks before copying and transactional publication.</summary>
/// <remarks>Indefinite lengths, unknown members, duplicates and unbounded tree migrations are not accepted on this wire path.</remarks>
template<class T> BoundedSerializationResult DeserializeBoundedCbor(const std::uint8_t* data, std::size_t size, T& object) {
    static_assert(IsBoundedSerializable<T>,"Bounded CBOR requires an explicit nonzero version and finite graph");
    static_assert(std::is_nothrow_move_assignable_v<T>,"Bounded decode publication requires nonthrowing move assignment");
    if (size>MaximumSerializedSize<T,CBOR>) return {SerializationErrorCode::ResourceLimitExceeded,0};
    if (!data || !size) return {SerializationErrorCode::MalformedInput,0};
    T candidate=object;
    DirectBinaryDetail::BoundedReader<CBOR> reader{data,data+size};
    if (!reader.Object(candidate)) return {reader.Error,0};
    if (reader.Cursor!=reader.End) return {SerializationErrorCode::MalformedInput,0};
    object=std::move(candidate); return {SerializationErrorCode::None,size};
}
}
