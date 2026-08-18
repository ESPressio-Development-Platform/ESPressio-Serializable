#include <Arduino.h>
#include <unity.h>
#include <ESPressio_Serializable_Arduino.hpp>
#include <ESPressio_Serializable_JSONStream.hpp>
using namespace ESPressio;
class MemoryStream:public Stream{String _s;size_t _p=0;public:explicit MemoryStream(const char*s):_s(s){}int available()override{return int(_s.length()-_p);}int read()override{return _p<_s.length()?_s[_p++]:-1;}int peek()override{return _p<_s.length()?_s[_p]:-1;}void flush()override{}size_t write(uint8_t)override{return 0;}};
class P:public Serializable::Serializable<P>{ESPRESSIO_SERIALIZABLE_TYPE(P) private:int _x=0;public:ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY_REQUIRED("x",_x))};
void malformed(){MemoryStream s("{bad json");Serializable::JsonStreamDeserializer d(s);P p;auto r=d.Deserialize(p);TEST_ASSERT_FALSE(bool(r));}
void filtered(){MemoryStream s("{\"x\":1,\"hugeUnused\":[1,2,3,4,5]}");Serializable::JsonStreamDeserializer d(s);P p;auto r=d.DeserializeSchemaFiltered(p);TEST_ASSERT_TRUE(bool(r));}
void setup(){UNITY_BEGIN();RUN_TEST(malformed);RUN_TEST(filtered);UNITY_END();}void loop(){}
