#include <cassert>
#include <string>
#include <ESPressio_Serializable.hpp>
#include <ESPressio_Migration.hpp>
using namespace ESPressio;
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members:
 * - _port (int): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 8 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class Current:public Serializable::Serializable<Current>{ESPRESSIO_SERIALIZABLE_TYPE(Current) ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(3) private:std::string _name;int _port=0; public:ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY_REQUIRED("name",_name),ESPRESSIO_PROPERTY("port",_port).Default(80)) static bool Migrate(Serializable::SerializationNode&n,uint32_t f,uint32_t to){if(f==1&&to==2)return Serializable::Migration::Rename(n,"deviceName","name");if(f==2&&to==3){auto* old=n.Find("httpPort");if(old){n.Set("port",*old);n.Remove("httpPort");}return true;}return false;} const std::string&Name()const{return _name;}int Port()const{return _port;}};
int main(){Serializable::TreeArchive v1;v1.Write("__schemaVersion",uint32_t(1));v1.Write("deviceName",std::string("legacy"));v1.Write("httpPort",int(8080));Current c;auto r=c.DeserializeDetailed(v1);assert(r);assert(c.Name()=="legacy");assert(c.Port()==8080);return 0;}
