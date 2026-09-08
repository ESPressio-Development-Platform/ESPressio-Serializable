#include <cassert>
#include <string>
#include <ESPressio_Serializable.hpp>
#include <ESPressio_SchemaIntrospection.hpp>
using namespace ESPressio;
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<S>) [0 bytes dynamic allocation]
 * Members:
 * - _secret (std::string): 24 bytes [Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Total Memory: sizeof(Serializable::Serializable<S>) + 24 bytes known members [_secret: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class S:public Serializable::Serializable<S>{ESPRESSIO_SERIALIZABLE_TYPE(S) ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(2) private:int _port=80;std::string _secret;public:ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY_REQUIRED("port",_port).Default(80),ESPRESSIO_PROPERTY_SENSITIVE("secret",_secret).Alias("password"))};
int main(){auto j=Serializable::SchemaInspector<S>::Json();auto c=Serializable::SchemaInspector<S>::Csv();auto m=Serializable::SchemaInspector<S>::Mermaid();assert(j.find("\"version\":2")!=std::string::npos);assert(j.find("password")!=std::string::npos);assert(c.find("property,type")!=std::string::npos);assert(m.find("classDiagram")!=std::string::npos);return 0;}
