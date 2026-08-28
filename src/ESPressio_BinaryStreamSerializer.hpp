#pragma once
#if !__has_include(<Arduino.h>)
#error "ESPressio_BinaryStreamSerializer.hpp requires Arduino Print."
#endif
#include <Arduino.h>
#include <cstring>
#include "ESPressio_Serializable.hpp"
namespace ESPressio::Serializable {
/// <summary>Serializes ESPressio values directly to an Arduino <c>Print</c> binary stream.</summary>
/// <typeparam name="IncludePropertyNames">When <c>true</c>, object property names are emitted; when <c>false</c>, the compact nameless object format is used.</typeparam>
template<bool IncludePropertyNames=true> class BinaryStreamSerializer {
 Print&_o; void u16(uint16_t v){_o.write((uint8_t)v);_o.write((uint8_t)(v>>8));} void u32(uint32_t v){for(int s=0;s<32;s+=8)_o.write((uint8_t)(v>>s));} void u64(uint64_t v){for(int s=0;s<64;s+=8)_o.write((uint8_t)(v>>s));}
 void name(const char*n){if constexpr(IncludePropertyNames){size_t z=n?strlen(n):0;u16((uint16_t)z);if(z)_o.write((const uint8_t*)n,z);}}
 template<class V> void val(const V&v){using U=std::decay_t<V>;if constexpr(HasSerializationAdapter<U>) node(SerializationAdapter<U>::ToNode(v));else if constexpr(IsSerializable<U>)obj(v);else if constexpr(Detail::IsStdOptional<U>::value){if(v)val(*v);else _o.write((uint8_t)SerializationNodeType::Null);}else if constexpr(Detail::IsSequence<U>){_o.write((uint8_t)SerializationNodeType::Array);u32(v.size());for(auto&i:v)val(i);}else if constexpr(Detail::IsMapLike<U>){_o.write((uint8_t)SerializationNodeType::Array);u32(v.size());for(auto&i:v){_o.write((uint8_t)SerializationNodeType::Object);u16(2);name("key");val(i.first);name("value");val(i.second);}}else if constexpr(std::is_same_v<U,bool>){_o.write((uint8_t)SerializationNodeType::Boolean);_o.write((uint8_t)(v?1:0));}else if constexpr(std::is_integral_v<U>&&std::is_signed_v<U>){_o.write((uint8_t)SerializationNodeType::SignedInteger);u64((uint64_t)(int64_t)v);}else if constexpr(std::is_integral_v<U>){_o.write((uint8_t)SerializationNodeType::UnsignedInteger);u64((uint64_t)v);}else if constexpr(std::is_same_v<U,float>){_o.write((uint8_t)SerializationNodeType::Float32);uint32_t x;memcpy(&x,&v,4);u32(x);}else if constexpr(std::is_floating_point_v<U>){_o.write((uint8_t)SerializationNodeType::Float64);uint64_t x;double d=v;memcpy(&x,&d,8);u64(x);}else if constexpr(std::is_same_v<U,std::string>){_o.write((uint8_t)SerializationNodeType::String);u32(v.size());_o.write((const uint8_t*)v.data(),v.size());}else if constexpr(std::is_enum_v<U>) val((std::underlying_type_t<U>)v);else static_assert(Detail::DependentFalse<U>::value,"Unsupported binary stream type");}
 void node(const SerializationNode&n){switch(n.GetType()){case SerializationNodeType::Null:_o.write((uint8_t)SerializationNodeType::Null);break;case SerializationNodeType::Boolean:val(n.BooleanValue());break;case SerializationNodeType::SignedInteger:val(n.SignedIntegerValue());break;case SerializationNodeType::UnsignedInteger:val(n.UnsignedIntegerValue());break;case SerializationNodeType::Float32:val(n.Float32Value());break;case SerializationNodeType::Float64:val(n.Float64Value());break;case SerializationNodeType::String:val(n.StringValue());break;default: static_assert(true,"containers handled by normal object traversal");break;}}
 template<class T> void obj(const T&o){_o.write((const uint8_t*)(IncludePropertyNames?"ESPN":"ESPN"),4);_o.write((uint8_t)(IncludePropertyNames?1:2));u32(T::GetSchemaVersion());constexpr size_t count=std::tuple_size<decltype(T::GetSerializableProperties())>::value;u16((uint16_t)count);std::apply([&](const auto&...p){((name(p.GetName()),val(p.GetValue(o))),...);},T::GetSerializableProperties());}
public:
 /// <summary>Creates a serializer that writes directly to the supplied Arduino output stream.</summary>
 explicit BinaryStreamSerializer(Print&o):_o(o){}
 /// <summary>Serializes an ESPressio-serializable object directly to the configured binary stream.</summary>
 template<class T>void Serialize(const T&o){obj(o);} };
/// <summary>Binary stream serializer variant that omits property names from object records.</summary>
using NamelessBinaryStreamSerializer=BinaryStreamSerializer<false>;
}
