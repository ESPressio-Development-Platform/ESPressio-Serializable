#pragma once
#if !__has_include(<Arduino.h>)
#error "ESPressio_CborStreamSerializer.hpp requires Arduino Print."
#endif
#include <Arduino.h>
#include <cstring>
#include "ESPressio_Serializable.hpp"
namespace ESPressio::Serializable {
/// <summary>Serializes ESPressio values directly to an Arduino <c>Print</c> stream using CBOR encoding.</summary>
class CborStreamSerializer {
 Print&_o; SerializationPolicy _policy;
 void arg(uint8_t m,uint64_t v){uint8_t p=m<<5;if(v<24){_o.write(p|v);}else if(v<=255){_o.write(p|24);_o.write((uint8_t)v);}else if(v<=65535){_o.write(p|25);_o.write((uint8_t)(v>>8));_o.write((uint8_t)v);}else{_o.write(p|27);for(int s=56;s>=0;s-=8)_o.write((uint8_t)(v>>s));}}
 void text(const char*s){size_t n=s?strlen(s):0;arg(3,n);if(n)_o.write((const uint8_t*)s,n);}
 template<class V> void val(const V&v){using U=std::decay_t<V>; if constexpr(HasSerializationAdapter<U>) node(SerializationAdapter<U>::ToNode(v)); else if constexpr(IsSerializable<U>) obj(v); else if constexpr(Detail::IsStdOptional<U>::value){if(v)val(*v);else _o.write((uint8_t)0xF6);} else if constexpr(Detail::IsSequence<U>){arg(4,v.size());for(auto&i:v)val(i);} else if constexpr(Detail::IsMapLike<U>){arg(4,v.size());for(auto&i:v){arg(5,2);text("key");val(i.first);text("value");val(i.second);}} else if constexpr(std::is_enum_v<U>){if constexpr(HasEnumSerializationMapping<U>){auto*n=EnumToString(v);if(n){text(n);return;}}val((std::underlying_type_t<U>)v);} else if constexpr(std::is_same_v<U,bool>)_o.write((uint8_t)(v?0xF5:0xF4)); else if constexpr(std::is_integral_v<U>){if constexpr(std::is_signed_v<U>){if(v<0)arg(1,(uint64_t)(-1-(int64_t)v));else arg(0,(uint64_t)v);}else arg(0,(uint64_t)v);} else if constexpr(std::is_same_v<U,float>){_o.write((uint8_t)0xFA);uint32_t x;memcpy(&x,&v,4);for(int s=24;s>=0;s-=8)_o.write((uint8_t)(x>>s));} else if constexpr(std::is_floating_point_v<U>){_o.write((uint8_t)0xFB);uint64_t x;double d=v;memcpy(&x,&d,8);for(int s=56;s>=0;s-=8)_o.write((uint8_t)(x>>s));} else if constexpr(std::is_same_v<U,std::string>)text(v.c_str()); else static_assert(Detail::DependentFalse<U>::value,"Unsupported CBOR stream type");}
 void node(const SerializationNode&n){switch(n.GetType()){case SerializationNodeType::Null:_o.write((uint8_t)0xF6);break;case SerializationNodeType::Boolean:val(n.BooleanValue());break;case SerializationNodeType::SignedInteger:val(n.SignedIntegerValue());break;case SerializationNodeType::UnsignedInteger:val(n.UnsignedIntegerValue());break;case SerializationNodeType::Float32:val(n.Float32Value());break;case SerializationNodeType::Float64:val(n.Float64Value());break;case SerializationNodeType::String:text(n.StringValue().c_str());break;case SerializationNodeType::Array:arg(4,n.ArrayChildren().size());for(auto&x:n.ArrayChildren())node(x);break;case SerializationNodeType::Object:arg(5,n.ObjectChildren().size());for(auto&x:n.ObjectChildren()){text(x.first.c_str());node(x.second);}break;}}
 template<class T> void obj(const T&o){size_t count=1;std::apply([&](const auto&...p){((count+=!(p.IsSensitive()&&_policy.GetSensitivePolicy()==SensitivePropertyPolicy::Omit)),...);},T::GetSerializableProperties());arg(5,count);text("__schemaVersion");val(T::GetSchemaVersion());std::apply([&](const auto&...p){(( [&](){if(p.IsSensitive()&&_policy.GetSensitivePolicy()==SensitivePropertyPolicy::Omit)return;text(p.GetName()?p.GetName():"");if(p.IsSensitive()&&_policy.GetSensitivePolicy()==SensitivePropertyPolicy::Redact)text(_policy.GetRedactionText());else val(p.GetValue(o));}()),...);},T::GetSerializableProperties());}
public:
 /// <summary>Creates a CBOR serializer writing directly to the supplied Arduino output stream.</summary>
 explicit CborStreamSerializer(Print&o):_o(o){}
 /// <summary>Returns mutable access to the sensitive-property serialization policy.</summary>
 SerializationPolicy& Policy(){return _policy;}
 /// <summary>Serializes an ESPressio-serializable object directly to CBOR.</summary>
 template<class T> void Serialize(const T&o){obj(o);} };
}
