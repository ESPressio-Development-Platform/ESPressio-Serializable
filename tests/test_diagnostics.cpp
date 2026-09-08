#include <cassert>
#include <cstdint>
#include <vector>
#include <ESPressio_Serializable.hpp>
using namespace ESPressio;
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (standalone empty object occupies 1 byte; an eligible empty base may be optimized to 0 bytes).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class Child:public Serializable::Serializable<Child>{ESPRESSIO_SERIALIZABLE_TYPE(Child) private:uint8_t _port=0; public:ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY_REQUIRED("port",_port))};
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (standalone empty object occupies 1 byte; an eligible empty base may be optimized to 0 bytes).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class Parent:public Serializable::Serializable<Parent>{ESPRESSIO_SERIALIZABLE_TYPE(Parent) private:std::vector<Child> _children; public:ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY_REQUIRED("children",_children))};
static Serializable::TreeArchive Invalid(){Serializable::TreeArchive a;Serializable::SerializationNode arr(Serializable::SerializationNodeType::Array);for(int i=0;i<2;++i){Serializable::SerializationNode obj(Serializable::SerializationNodeType::Object);obj.Set("port",Serializable::Detail::ToNode(int64_t(1000+i)));arr.Append(std::move(obj));}a.GetNode().Set("children",std::move(arr));return a;}
int main(){Parent p;auto a=Invalid();Serializable::DeserializationOptions all;all.Behavior=Serializable::ValidationBehavior::CollectAll;auto r=p.DeserializeDetailed(a,all);assert(!r);assert(r.IssueCount()==2);assert(r.Issues()[0].Path=="children[0].port");auto b=Invalid();Serializable::DeserializationOptions ff;ff.Behavior=Serializable::ValidationBehavior::FailFast;auto r2=p.DeserializeDetailed(b,ff);assert(!r2);assert(r2.IssueCount()==1);return 0;}
