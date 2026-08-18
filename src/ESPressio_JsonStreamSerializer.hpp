#pragma once
#if !__has_include(<Arduino.h>)
#error "ESPressio_JsonStreamSerializer.hpp requires Arduino Print/Stream."
#endif
#include <Arduino.h>
#include <cstdio>
#include <string>
#include "ESPressio_Serializable.hpp"

namespace ESPressio::Serializable {

    class JsonStreamSerializer {
        private:
            Print& _output;
            SerializationPolicy _policy;

            void StringValue(const char* text) {
                _output.write('"');
                if (text != nullptr) {
                    for (const char* p=text; *p; ++p) {
                        switch (*p) {
                            case '"': _output.print("\\\""); break;
                            case '\\': _output.print("\\\\"); break;
                            case '\n': _output.print("\\n"); break;
                            case '\r': _output.print("\\r"); break;
                            case '\t': _output.print("\\t"); break;
                            default: _output.write(static_cast<uint8_t>(*p)); break;
                        }
                    }
                }
                _output.write('"');
            }

            template<typename T> void Value(const T& value) {
                using U=std::remove_cv_t<std::remove_reference_t<T>>;
                if constexpr (HasSerializationAdapter<U>) {
                    NodeValue(SerializationAdapter<U>::ToNode(value));
                } else if constexpr (IsSerializable<U>) {
                    Object(value);
                } else if constexpr (Detail::IsStdOptional<U>::value) {
                    if (value) Value(*value); else _output.print("null");
                } else if constexpr (Detail::IsSequence<U> || Detail::IsMapLike<U>) {
                    Array(value);
                } else if constexpr (std::is_enum_v<U>) {
                    if constexpr (HasEnumSerializationMapping<U>) {
                        const char* name=EnumToString(value);
                        if(name){ StringValue(name); return; }
                    }
                    Value(static_cast<std::underlying_type_t<U>>(value));
                } else if constexpr (std::is_same_v<U,bool>) {
                    _output.print(value?"true":"false");
                } else if constexpr (std::is_integral_v<U>) {
                    char b[32]; if constexpr(std::is_signed_v<U>) std::snprintf(b,sizeof(b),"%lld",static_cast<long long>(value)); else std::snprintf(b,sizeof(b),"%llu",static_cast<unsigned long long>(value)); _output.print(b);
                } else if constexpr (std::is_floating_point_v<U>) {
                    char b[48]; std::snprintf(b,sizeof(b),"%.17g",static_cast<double>(value)); _output.print(b);
                } else if constexpr (std::is_same_v<U,std::string>) {
                    StringValue(value.c_str());
                } else {
                    static_assert(Detail::DependentFalse<U>::value,"ESPressio JsonStreamSerializer: unsupported type.");
                }
            }

            void NodeValue(const SerializationNode& n) {
                switch(n.GetType()) {
                    case SerializationNodeType::Null: _output.print("null"); break;
                    case SerializationNodeType::Boolean: _output.print(n.BooleanValue()?"true":"false"); break;
                    case SerializationNodeType::SignedInteger: Value(n.SignedIntegerValue()); break;
                    case SerializationNodeType::UnsignedInteger: Value(n.UnsignedIntegerValue()); break;
                    case SerializationNodeType::Float32: Value(n.Float32Value()); break;
                    case SerializationNodeType::Float64: Value(n.Float64Value()); break;
                    case SerializationNodeType::String: StringValue(n.StringValue().c_str()); break;
                    case SerializationNodeType::Array: { _output.write('['); bool f=true; for(auto& c:n.ArrayChildren()){if(!f)_output.write(',');f=false;NodeValue(c);} _output.write(']'); break; }
                    case SerializationNodeType::Object: { _output.write('{'); bool f=true; for(auto& c:n.ObjectChildren()){if(!f)_output.write(',');f=false;StringValue(c.first.c_str());_output.write(':');NodeValue(c.second);} _output.write('}'); break; }
                }
            }

            template<typename C> void Array(const C& collection) {
                _output.write('['); bool first=true;
                if constexpr (Detail::IsMapLike<C>) {
                    for(const auto& item:collection){ if(!first)_output.write(','); first=false; _output.print("{\"key\":");Value(item.first);_output.print(",\"value\":");Value(item.second);_output.write('}'); }
                } else {
                    for(const auto& item:collection){ if(!first)_output.write(','); first=false; Value(item); }
                }
                _output.write(']');
            }

            template<typename T> void Object(const T& object) {
                _output.write('{'); bool first=true;
                auto emitName=[&](const char* n){if(!first)_output.write(',');first=false;StringValue(n);_output.write(':');};
                emitName("__schemaVersion"); Value(T::GetSchemaVersion());
                std::apply([&](const auto&... property){(( [&](){
                    if(property.IsSensitive() && _policy.GetSensitivePolicy()==SensitivePropertyPolicy::Omit) return;
                    emitName(property.GetName());
                    if(property.IsSensitive() && _policy.GetSensitivePolicy()==SensitivePropertyPolicy::Redact) StringValue(_policy.GetRedactionText());
                    else Value(property.GetValue(object));
                }() ),...);},T::GetSerializableProperties());
                _output.write('}');
            }

        public:
            explicit JsonStreamSerializer(Print& output):_output(output){}
            SerializationPolicy& Policy(){return _policy;}
            template<typename T> void Serialize(const T& object){Object(object);}
    };
}
