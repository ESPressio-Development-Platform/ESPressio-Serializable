#define ESPRESSIO_SERIALIZATION_STATIC_POOL_BYTES 32768
#include <ESPressio_PoolAllocator.hpp>
#define ESPRESSIO_SERIALIZATION_ALLOCATOR ESPressio::Serializable::StaticPoolAllocator
#include <ESPressio_Serializable.hpp>
#include <ESPressio_CborArchive.hpp>
#include <cassert>
using namespace ESPressio;
class P:public Serializable::Serializable<P>{ESPRESSIO_SERIALIZABLE_TYPE(P) private:int _x=1; public:ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("x",_x))};
int main(){Serializable::SerializationStaticPool().Reset();P p;Serializable::CborArchive a;p.Serialize(a);auto d=a.GetData();assert(!d.empty());assert(Serializable::SerializationStaticPool().Used()>0);return 0;}
