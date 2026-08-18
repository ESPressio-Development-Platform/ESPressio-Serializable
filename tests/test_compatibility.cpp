#include <cassert>
#include <iomanip>
#include <sstream>
#include <string>
#include <ESPressio_Serializable.hpp>
#include <ESPressio_BinaryArchive.hpp>
#include <ESPressio_CborArchive.hpp>
using namespace ESPressio;
class VectorState:public Serializable::Serializable<VectorState>{ESPRESSIO_SERIALIZABLE_TYPE(VectorState) ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(2) private:uint32_t _counter=42;std::string _name="vector";bool _enabled=true;public:ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("counter",_counter),ESPRESSIO_PROPERTY("name",_name),ESPRESSIO_PROPERTY("enabled",_enabled))};
template<class A> std::string Hex(){VectorState s;A a;s.Serialize(a);auto d=a.GetData();std::ostringstream o;for(auto b:d)o<<std::hex<<std::setfill('0')<<std::setw(2)<<(unsigned)b;return o.str();}
int main(){assert(Hex<Serializable::BinaryArchive>()=="45535042020104000f005f5f736368656d6156657273696f6e0502000000000000000700636f756e746572052a0000000000000004006e616d650806000000766563746f720700656e61626c65640301");assert(Hex<Serializable::CborArchive>()=="a46f5f5f736368656d6156657273696f6e0267636f756e746572182a646e616d6566766563746f7267656e61626c6564f5");return 0;}
