#pragma once
#include <sstream>
#include <string>
#include <vector>
#include <type_traits>
#include "ESPressio_Serializable.hpp"
namespace ESPressio::Serializable {
struct PropertySchemaInfo { std::string Name; bool Required=false, ReadOnly=false, Sensitive=false, HasDefault=false; std::vector<std::string> Aliases; std::string Type; };
namespace Detail {
template<typename T> std::string SchemaTypeName(){ using U=std::remove_cv_t<std::remove_reference_t<T>>; if constexpr(IsSerializable<U>) return "object"; else if constexpr(IsStdOptional<U>::value) return "optional"; else if constexpr(IsSequence<U>) return "array"; else if constexpr(IsMapLike<U>) return "map"; else if constexpr(std::is_enum_v<U>) return "enum"; else if constexpr(std::is_same_v<U,bool>) return "boolean"; else if constexpr(std::is_integral_v<U>) return "integer"; else if constexpr(std::is_floating_point_v<U>) return "number"; else if constexpr(IsStdString<U>) return "string"; else return "custom"; }
inline std::string JsonEscape(const std::string&s){std::string o;for(char c:s){if(c=='"'||c=='\\')o+='\\';o+=c;}return o;}
inline std::string CsvEscape(const std::string&s){std::string o="\"";for(char c:s){if(c=='\"')o+='\"';o+=c;}return o+'\"';}
}
template<typename T> class SchemaInspector { public:
 static std::vector<PropertySchemaInfo> Properties(){ std::vector<PropertySchemaInfo> out; std::apply([&](const auto&... p){(( [&](){ PropertySchemaInfo i; i.Name=p.GetName()?p.GetName():""; i.Required=p.IsRequired(); i.ReadOnly=p.IsReadOnly(); i.Sensitive=p.IsSensitive(); i.HasDefault=p.HasDefault(); for(size_t x=0;x<p.GetAliasCount();++x) if(p.GetAlias(x)) i.Aliases.emplace_back(p.GetAlias(x)); i.Type=Detail::SchemaTypeName<typename std::decay_t<decltype(p)>::ValueType>(); out.push_back(std::move(i)); }() ),...);},T::GetSerializableProperties()); return out; }
 static std::string Markdown(){ std::ostringstream o; o<<"# Schema\n\nVersion: "<<T::GetSchemaVersion()<<"\n\n| Property | Type | Required | Read-only | Sensitive | Aliases |\n|---|---|---:|---:|---:|---|\n"; for(auto&i:Properties()){o<<"| "<<i.Name<<" | "<<i.Type<<" | "<<(i.Required?"yes":"no")<<" | "<<(i.ReadOnly?"yes":"no")<<" | "<<(i.Sensitive?"yes":"no")<<" | "; for(size_t x=0;x<i.Aliases.size();++x){if(x)o<<", ";o<<i.Aliases[x];} o<<" |\n";} return o.str(); }
 static std::string Json(){std::ostringstream o;o<<"{\"version\":"<<T::GetSchemaVersion()<<",\"properties\":[";bool first=true;for(auto&i:Properties()){if(!first)o<<',';first=false;o<<"{\"name\":\""<<Detail::JsonEscape(i.Name)<<"\",\"type\":\""<<i.Type<<"\",\"required\":"<<(i.Required?"true":"false")<<",\"readOnly\":"<<(i.ReadOnly?"true":"false")<<",\"sensitive\":"<<(i.Sensitive?"true":"false")<<",\"hasDefault\":"<<(i.HasDefault?"true":"false")<<",\"aliases\":[";for(size_t x=0;x<i.Aliases.size();++x){if(x)o<<',';o<<'\"'<<Detail::JsonEscape(i.Aliases[x])<<'\"';}o<<"]}";}o<<"]}";return o.str();}
 static std::string Csv(){std::ostringstream o;o<<"property,type,required,read_only,sensitive,has_default,aliases\n";for(auto&i:Properties()){std::string a;for(size_t x=0;x<i.Aliases.size();++x){if(x)a+=';';a+=i.Aliases[x];}o<<Detail::CsvEscape(i.Name)<<','<<Detail::CsvEscape(i.Type)<<','<<(i.Required?1:0)<<','<<(i.ReadOnly?1:0)<<','<<(i.Sensitive?1:0)<<','<<(i.HasDefault?1:0)<<','<<Detail::CsvEscape(a)<<"\n";}return o.str();}
 static std::string Mermaid(){std::ostringstream o;o<<"classDiagram\nclass SerializableSchema {\n  +schemaVersion "<<T::GetSchemaVersion()<<"\n}\n";for(auto&i:Properties())o<<"SerializableSchema : +"<<i.Type<<" "<<i.Name<<(i.Required?" [required]":"")<<"\n";return o.str();}
};
}
