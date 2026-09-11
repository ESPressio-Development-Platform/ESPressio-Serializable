#include <ESPressio_Serializable.hpp>
#include <cassert>
#include <array>
#include <limits>
#include <random>
using namespace ESPressio::Serializable;

struct Edge : Serializable<Edge> {
    std::optional<std::optional<int>> Nested;
    BoundedString<8> Text;
    double Number=0;
    std::uint64_t ReadOnly=0;
    ESPRESSIO_SERIALIZABLE_TYPE(Edge)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(3)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("nested",Nested),ESPRESSIO_PROPERTY("text",Text),
        ESPRESSIO_PROPERTY("number",Number),ESPRESSIO_PROPERTY("readonly",ReadOnly).ReadOnly())
};
struct Duplicate {
    int First{}, Second{};
    ESPRESSIO_SERIALIZABLE_TYPE(Duplicate)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("first",First).Alias("second"),ESPRESSIO_PROPERTY("second",Second))
};
struct AliasOverflow {
    int Value{};
    ESPRESSIO_SERIALIZABLE_TYPE(AliasOverflow)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("value",Value).Alias("a").Alias("b").Alias("c").Alias("d").Alias("e"))
};
static_assert(!IsBoundedSerializable<Duplicate> && !IsBoundedSerializable<AliasOverflow>);
static bool Json(std::string_view text, Edge& output) {
    return bool(DeserializeBoundedJson(reinterpret_cast<const std::uint8_t*>(text.data()),text.size(),output));
}
static void Check(const Edge& input) {
    std::array<std::uint8_t,MaximumSerializedSize<Edge,DirectBinary>> binary{};
    std::array<std::uint8_t,MaximumSerializedSize<Edge,CBOR>> cbor{};
    std::array<std::uint8_t,MaximumSerializedSize<Edge,JSON>> json{};
    Edge b,c,j;
    auto eb=SerializeDirectBinary(input,binary.data(),binary.size()); assert(eb);
    auto ec=SerializeBoundedCbor(input,cbor.data(),cbor.size()); assert(ec);
    auto ej=SerializeBoundedJson(input,json.data(),json.size()); assert(ej);
    assert(DeserializeBoundedDirectBinary(binary.data(),eb.Bytes,b));
    assert(DeserializeBoundedCbor(cbor.data(),ec.Bytes,c));
    assert(DeserializeBoundedJson(json.data(),ej.Bytes,j));
    for (const auto* value:std::array<const Edge*,3>{&b,&c,&j}) {
        assert(value->Nested==input.Nested && value->Text==input.Text);
        assert(value->Number==input.Number && value->ReadOnly==input.ReadOnly);
    }
}
int main() {
    Edge value;
    value.ReadOnly=std::numeric_limits<std::uint64_t>::max();
    Check(value);
    value.Nested.emplace(); Check(value); // present outer, absent inner
    value.Nested->emplace(42); Check(value);
    for (double d:std::array<double,6>{-0.0,0.1,-0.1,std::numeric_limits<double>::min(),std::numeric_limits<double>::denorm_min(),std::numeric_limits<double>::max()}) {
        value.Number=d; Check(value);
    }
    assert(value.Text.assign("\xf0\x9f\x98\x80")); Check(value);
    assert(Json("{\"__schemaVersion\":3,\"text\":\"\\uD83D\\uDE00\",\"readonly\":12}",value));
    assert(value.Text.view()=="\xf0\x9f\x98\x80" && value.ReadOnly==12);
    assert(Json("{\"__schemaVersion\":3,\"te\\u0078t\":\"\\u0000\"}",value));
    assert(value.Text.size()==1 && value.Text.data()[0]==0);
    {
        Edge invalid;
        assert(invalid.Text.assign("\xc0\xaf")); // overlong UTF-8 must not enter a text wire format
        std::array<std::uint8_t,MaximumSerializedSize<Edge,JSON>> bytes{};
        assert(!SerializeBoundedJson(invalid,bytes.data(),bytes.size()));
        assert(!SerializeBoundedCbor(invalid,bytes.data(),bytes.size()));
    }
    for (auto malformed:std::array<std::string_view,12>{
        "{\"__schemaVersion\":2}","{\"__schemaVersion\":3,\"text\":\"\\uD800\"}",
        "{\"__schemaVersion\":3,\"text\":\"\\uDC00\"}","{\"__schemaVersion\":3,\"text\":\"\\q\"}",
        "{\"__schemaVersion\":3,\"text\":\"123456789\"}","{\"__schemaVersion\":3,\"number\":01}",
        "{\"__schemaVersion\":3,\"number\":1e}","{\"__schemaVersion\":3,\"number\":1e9999}",
        "{\"__schemaVersion\":3,\"readonly\":-1}","{\"__schemaVersion\":3,\"readonly\":18446744073709551616}",
        "{\"__schemaVersion\":3,\"text\":\"a\",\"text\":\"b\"}","{\"__schemaVersion\":3,\"unknown\":{}}"}) {
        assert(!Json(malformed,value)); assert(value.Text.size()==1 && value.Text.data()[0]==0);
    }
    // Deterministic property fuzz: every generated value must fit the derived maxima in all formats.
    std::mt19937_64 random(0x5033463036ull);
    for (unsigned i=0;i<2000;++i) {
        value.Text.clear(); for (unsigned n=0;n<i%9;++n) assert(value.Text.push_back(static_cast<char>(random()%128)));
        value.Nested.emplace(static_cast<int>(random())); value.ReadOnly=random();
        value.Number=static_cast<double>(static_cast<std::int64_t>(random()))/97.0;
        Check(value);
    }
}
