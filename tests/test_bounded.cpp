#include <ESPressio_BoundedDeserializer.hpp>
#include <ESPressio_SerializationMacros.hpp>
#include <ESPressio_SerializableBase.hpp>
#include <ESPressio_CborArchive.hpp>
#include <ESPressio_BoundedCborArchive.hpp>
#include <ESPressio_BoundedJsonArchive.hpp>
#include <cassert>
#include <cstdlib>
#include <new>
#include <string>

using namespace ESPressio::Serializable;
static bool heapForbidden = false;
void* operator new(std::size_t size) {
    if (heapForbidden) std::abort();
    if (auto* value = std::malloc(size ? size : 1)) return value;
    throw std::bad_alloc();
}
void operator delete(void* value) noexcept { std::free(value); }
void operator delete(void* value, std::size_t) noexcept { std::free(value); }

struct Child : Serializable<Child> {
    std::int16_t Number = 7;
    ESPRESSIO_SERIALIZABLE_TYPE(Child)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(2)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("number", Number).Required().Default(7).Range(-20,20))
};
struct Sample : Serializable<Sample> {
    BoundedString<12> Text;
    BoundedVector<Child,3> Children;
    BoundedBytes<4> Bytes;
    BoundedMap<BoundedString<4>,std::uint32_t,2> Map;
    BoundedSet<std::uint8_t,3> Set;
    std::optional<std::array<std::uint16_t,2>> Optional;
    std::variant<std::int32_t,BoundedString<5>> Choice;
    ESPRESSIO_SERIALIZABLE_TYPE(Sample)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("text",Text).Sensitive().Alias("previousText"),
        ESPRESSIO_PROPERTY("children",Children), ESPRESSIO_PROPERTY("bytes",Bytes),
        ESPRESSIO_PROPERTY("map",Map),ESPRESSIO_PROPERTY("set",Set),
        ESPRESSIO_PROPERTY("optional",Optional),ESPRESSIO_PROPERTY("choice",Choice))
};
struct Dynamic {
    std::string Text;
    ESPRESSIO_SERIALIZABLE_TYPE(Dynamic)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("text",Text))
};
struct MissingVersion {
    int Value{};
    ESPRESSIO_SERIALIZABLE_TYPE(MissingVersion)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("value",Value))
};
struct OrderA {
    int A{}; bool B{};
    ESPRESSIO_SERIALIZABLE_TYPE(OrderA)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("a",A).Alias("alpha").Default(3), ESPRESSIO_PROPERTY("b",B))
};
struct OrderB {
    bool B{}; int A{};
    ESPRESSIO_SERIALIZABLE_TYPE(OrderB)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("b",B), ESPRESSIO_PROPERTY("a",A).Alias("alpha").Default(3))
};
struct DifferentDefault {
    int A{}; bool B{};
    ESPRESSIO_SERIALIZABLE_TYPE(DifferentDefault)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(ESPRESSIO_PROPERTY("a",A).Alias("alpha").Default(4),ESPRESSIO_PROPERTY("b",B))
};
static_assert(IsSerializable<Dynamic> && !IsBoundedSerializable<Dynamic>);
static_assert(!IsBoundedSerializable<MissingVersion> && !IsBoundedSerializable<int>);
static_assert(IsBoundedSerializable<Sample>);
static_assert(!BoundedValueTraits<BoundedVector<std::string,2>>::IsBounded);
static_assert(MaximumSerializedSize<Sample,JSON> > MaximumSerializedSize<Sample,DirectBinary>);

struct CanonicalBytes final {
    std::array<std::uint8_t,2048> Bytes{};
    std::size_t Size=0;
    constexpr void Byte(std::uint8_t value) noexcept { if(Size==Bytes.size()) std::abort(); Bytes[Size++]=value; }
    constexpr void Integer(std::uint64_t value) noexcept { for(unsigned i=0;i<8;++i) {Byte(static_cast<std::uint8_t>(value));value>>=8;} }
    constexpr void Text(std::string_view value) noexcept { Integer(value.size()); for(char c:value) Byte(static_cast<std::uint8_t>(c)); }
};
constexpr auto CanonicalOrderA=[] {CanonicalBytes bytes;WriteCanonicalSchema<OrderA>(bytes);return bytes;}();
static_assert(CanonicalOrderA.Size>32);
int main() {
    heapForbidden = true;
    CanonicalBytes canonicalB,canonicalDifferent;
    WriteCanonicalSchema<OrderB>(canonicalB);WriteCanonicalSchema<DifferentDefault>(canonicalDifferent);
    assert(CanonicalOrderA.Size==canonicalB.Size && CanonicalOrderA.Bytes==canonicalB.Bytes);
    assert(CanonicalOrderA.Bytes!=canonicalDifferent.Bytes);
    BoundedDetail::StructuralHash structural;
    WriteCanonicalSchema<Sample>(structural);
    assert(structural.Value==SchemaDescriptor<Sample>().StructuralFingerprint);
    BoundedVector<int,0> zero;
    assert(!zero.push_back(1) && zero.empty());
    BoundedString<4> text;
    assert(text.assign("abcd")); assert(!text.assign("abcde")); assert(text.view()=="abcd");
    assert(text.assign(text.view().substr(1))); assert(text.view()=="bcd" && text.c_str()[3]==0);
    BoundedSet<int,2> set;
    assert(set.insert(1) && !set.insert(1) && set.insert(2) && !set.insert(3));
    BoundedMap<int,int,1> map;
    assert(map.insert(1,2) && !map.insert(1,3) && !map.insert(2,3) && *map.find(1)==2);
    const auto& schema = SchemaDescriptor<Sample>();
    assert(schema.PropertyCount==7 && schema.CurrentVersion==1 && schema.MinimumReadableVersion==1);
    assert(schema.Properties[0].Alias(0)=="previousText" && schema.Properties[0].Alias(9).empty());
    assert(schema.Properties[1].Value->Element->NestedSchema->CurrentVersion==2);
    assert(schema.Properties[6].Value->AlternativeCount==2);
    assert(SchemaDescriptor<OrderA>().StructuralFingerprint==SchemaDescriptor<OrderB>().StructuralFingerprint);
    assert(SchemaDescriptor<OrderA>().StructuralFingerprint!=SchemaDescriptor<DifferentDefault>().StructuralFingerprint);

    Sample sample;
    assert(sample.Text.assign("secret\"\\\n"));
    for (int i=0;i<3;++i) assert(sample.Children.push_back(Child{}));
    for (int i=0;i<4;++i) assert(sample.Bytes.push_back(std::uint8_t(i)));
    BoundedString<4> key; assert(key.assign("abcd")); assert(sample.Map.insert(key,0xffffffffu));
    assert(key.assign("efgh")); assert(sample.Map.insert(key,0));
    assert(sample.Set.insert(0) && sample.Set.insert(1) && sample.Set.insert(255));
    sample.Optional = std::array<std::uint16_t,2>{0,65535};
    BoundedString<5> choice; assert(choice.assign("12345")); sample.Choice = choice;
    std::array<std::uint8_t,MaximumSerializedSize<Sample,DirectBinary>> output{};
    auto encoded = SerializeDirectBinary(sample,output.data(),output.size()); assert(encoded);
    Sample restored;
    auto decoded = DeserializeBoundedDirectBinary(output.data(),encoded.Bytes,restored); assert(decoded);
    assert(restored.Text==sample.Text && restored.Map==sample.Map && restored.Bytes==sample.Bytes && restored.Choice==sample.Choice);
    assert(restored.Children.size()==3 && restored.Optional==sample.Optional);
    // Every truncation must leave the destination unchanged.
    for (std::size_t n=0;n<encoded.Bytes;++n) {
        assert(!DeserializeBoundedDirectBinary(output.data(),n,restored));
        assert(restored.Text==sample.Text && restored.Children.size()==3);
    }
    assert(!DeserializeBoundedDirectBinary(output.data(),output.size()+1,restored));
    // Schema version is explicit; general tree migrations never run here.
    auto saved = output[26]; output[26] = 0;
    assert(!DeserializeBoundedDirectBinary(output.data(),encoded.Bytes,restored)); output[26] = saved;
    // Nested length lies inside a valid total-size envelope; reject before copying.
    auto bad = output;
    for (std::size_t i=5;i+9<encoded.Bytes;++i) {
        if (bad[i]==4 && bad[i+1]==0 && std::memcmp(bad.data()+i+2,"text",4)==0) {
            bad[i+7]=13; bad[i+8]=0; bad[i+9]=0; bad[i+10]=0; break;
        }
    }
    auto rejected = DeserializeBoundedDirectBinary(bad.data(),encoded.Bytes,restored);
    assert(!rejected && rejected.Error==SerializationErrorCode::ResourceLimitExceeded);
    std::array<std::uint8_t,MaximumSerializedSize<Sample,CBOR>> cborBytes{};
    auto encodedCbor=SerializeBoundedCbor(sample,cborBytes.data(),cborBytes.size()); assert(encodedCbor);
    Sample fromCbor;
    assert(DeserializeBoundedCbor(cborBytes.data(),encodedCbor.Bytes,fromCbor));
    assert(fromCbor.Text==sample.Text && fromCbor.Map==sample.Map && fromCbor.Choice==sample.Choice);
    for (std::size_t n=0;n<encodedCbor.Bytes;++n) assert(!DeserializeBoundedCbor(cborBytes.data(),n,fromCbor));
    heapForbidden = false;
    heapForbidden = true;
    std::array<std::uint8_t,MaximumSerializedSize<Sample,JSON>> jsonBytes{};
    auto encodedJson=SerializeBoundedJson(sample,jsonBytes.data(),jsonBytes.size()); assert(encodedJson);
    Sample fromJson;
    assert(DeserializeBoundedJson(jsonBytes.data(),encodedJson.Bytes,fromJson));
    assert(fromJson.Text==sample.Text && fromJson.Map==sample.Map && fromJson.Choice==sample.Choice);
    for (std::size_t n=0;n<encodedJson.Bytes;++n) assert(!DeserializeBoundedJson(jsonBytes.data(),n,fromJson));
    heapForbidden = false;
    // Same metadata and encoder produce the existing general archive representation.
    std::vector<std::uint8_t> general;
    assert(SerializeDirectBinary(sample,general));
    assert(general.size()==encoded.Bytes && std::memcmp(general.data(),output.data(),general.size())==0);
    CborArchive cbor; sample.Serialize(cbor);
    assert((cbor.GetData().size()<=MaximumSerializedSize<Sample,CBOR>));
    const auto generalCbor=cbor.GetData();
    assert(generalCbor.size()==encodedCbor.Bytes && std::memcmp(generalCbor.data(),cborBytes.data(),generalCbor.size())==0);
}
