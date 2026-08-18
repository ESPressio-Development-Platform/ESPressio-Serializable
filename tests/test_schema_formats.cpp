#include <cassert>
#include <string>
#include <ESPressio_Serializable.hpp>
#include <ESPressio_SchemaIntrospection.hpp>
using namespace ESPressio;
class S:public Serializable::Serializable<S>{ESPRESSIO_SERIALIZABLE_TYPE(S) ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(2) private:int _port=80;std::string _secret;public:ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY_REQUIRED("port",_port).Default(80),ESPRESSIO_PROPERTY_SENSITIVE("secret",_secret).Alias("password"))};
int main(){auto j=Serializable::SchemaInspector<S>::Json();auto c=Serializable::SchemaInspector<S>::Csv();auto m=Serializable::SchemaInspector<S>::Mermaid();assert(j.find("\"version\":2")!=std::string::npos);assert(j.find("password")!=std::string::npos);assert(c.find("property,type")!=std::string::npos);assert(m.find("classDiagram")!=std::string::npos);return 0;}
