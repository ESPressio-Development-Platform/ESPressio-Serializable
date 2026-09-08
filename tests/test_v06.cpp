#include <cassert>
#include <string>
#include <ESPressio_Serializable.hpp>
using namespace ESPressio;
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: sizeof(Serializable::Serializable<V6>) [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: sizeof(Serializable::Serializable<V6>) [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: low; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class V6 : public Serializable::Serializable<V6> {
 ESPRESSIO_SERIALIZABLE_TYPE(V6)
 int _x=4;
 public:
 ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY_REQUIRED("x",_x).Default(7))
};
int main(){
 auto md=Serializable::SchemaInspector<V6>::Markdown(); assert(md.find("x")!=std::string::npos);
 Serializable::SerializationNode root(Serializable::SerializationNodeType::Object);
 Serializable::SerializationNode arr(Serializable::SerializationNodeType::Array);
 Serializable::SerializationNode item(Serializable::SerializationNodeType::Object); item.Set("name",Serializable::Detail::ToNode(std::string("a"))); arr.Append(std::move(item)); root.Set("items",std::move(arr));
 assert(Serializable::Migration::ResolvePath(root,"items[0].name")!=nullptr);
 assert(Serializable::Migration::RemoveAt(root,"items[0].name"));
 Serializable::TreeArchive a; V6 v; auto result=v.DeserializeDetailed(a); assert(result.Success());
}
