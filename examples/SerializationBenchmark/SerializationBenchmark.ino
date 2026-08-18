#include <Arduino.h>
#include <vector>
#include <ESPressio_Serializable_CBOR.hpp>
#include <ESPressio_Serializable_Binary.hpp>
#ifdef ESP32
#include <esp_heap_caps.h>
#endif
using namespace ESPressio;
class BenchmarkItem:public Serializable::Serializable<BenchmarkItem>{ESPRESSIO_SERIALIZABLE_TYPE(BenchmarkItem) private:uint32_t _id=0;float _value=0; public: BenchmarkItem()=default;BenchmarkItem(uint32_t i,float v):_id(i),_value(v){} ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("id",_id),ESPRESSIO_PROPERTY("value",_value))};
class BenchmarkPayload:public Serializable::Serializable<BenchmarkPayload>{ESPRESSIO_SERIALIZABLE_TYPE(BenchmarkPayload) private:std::vector<BenchmarkItem> _items; public:void Resize(size_t n){_items.clear();_items.reserve(n);for(size_t i=0;i<n;++i)_items.emplace_back(i,float(i)*0.25f);} ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("items",_items))};
template<class A> void Run(const char*name,size_t count){BenchmarkPayload p;p.Resize(count);uint32_t before=ESP.getFreeHeap();uint32_t start=micros();A a;p.Serialize(a);auto bytes=a.GetData();uint32_t elapsed=micros()-start;uint32_t after=ESP.getFreeHeap();Serial.printf("%s,%u,%u,%lu,%ld\n",name,(unsigned)count,(unsigned)bytes.size(),(unsigned long)elapsed,(long)before-(long)after);}
void setup(){Serial.begin(115200);delay(500);Serial.println("format,items,payload_bytes,serialize_us,heap_delta_bytes");for(size_t n:{1u,10u,100u,500u}){Run<Serializable::BinaryArchive>("binary",n);Run<Serializable::CborArchive>("cbor",n);}}
void loop(){}
