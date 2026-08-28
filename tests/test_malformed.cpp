#include <cassert>
#include <cstdint>
#include <random>
#include <string_view>
#include <vector>

#include <ESPressio_BinaryArchive.hpp>
#include <ESPressio_BinaryArchiveTraversal.hpp>
#include <ESPressio_CborArchive.hpp>

using namespace ESPressio;

template<class A, typename TAllocator>
void Mutate(const std::vector<uint8_t, TAllocator>& seed) {
    std::mt19937 rng(0x45535052);
    for (int n = 0; n < 2000; ++n) {
        std::vector<uint8_t> d(seed.begin(), seed.end());
        if (d.empty()) d.push_back(0);
        const int edits = 1 + (rng() % 4);
        for (int e = 0; e < edits; ++e) {
            switch (rng() % 3) {
                case 0:
                    d[rng() % d.size()] ^= uint8_t(1u << (rng() % 8));
                    break;
                case 1:
                    if (d.size() > 1) d.erase(d.begin() + (rng() % d.size()));
                    break;
                case 2:
                    if (d.size() < 2048) {
                        d.insert(
                            d.begin() + (rng() % (d.size() + 1)),
                            uint8_t(rng())
                        );
                    }
                    break;
            }
        }
        A a;
        (void)a.Load(d);
    }
}

static std::vector<uint8_t> Header() {
    return {'E', 'S', 'P', 'B', 2u};
}

static void AppendU16(std::vector<uint8_t>& data, uint16_t value) {
    data.push_back(static_cast<uint8_t>(value & 0xffu));
    data.push_back(static_cast<uint8_t>((value >> 8u) & 0xffu));
}

static void AppendU32(std::vector<uint8_t>& data, uint32_t value) {
    for (unsigned shift = 0; shift < 32; shift += 8) {
        data.push_back(static_cast<uint8_t>((value >> shift) & 0xffu));
    }
}

static std::vector<uint8_t> DeepObject(unsigned depth) {
    auto data = Header();
    for (unsigned level = 0; level < depth; ++level) {
        data.push_back(static_cast<uint8_t>(
            Serializable::SerializationNodeType::Object
        ));
        AppendU16(data, 1);
        AppendU16(data, 1);
        data.push_back('x');
    }
    data.push_back(static_cast<uint8_t>(
        Serializable::SerializationNodeType::Null
    ));
    return data;
}

class CountingVisitor final : public Serializable::BinaryArchiveVisitor {
public:
    unsigned Properties = 0;
    unsigned UnsignedValues = 0;

    bool OnObjectProperty(
        std::string_view,
        uint32_t,
        uint32_t,
        std::size_t
    ) noexcept override {
        ++Properties;
        return true;
    }

    bool OnUnsignedInteger(
        uint64_t,
        std::size_t
    ) noexcept override {
        ++UnsignedValues;
        return true;
    }
};

int main() {
    Serializable::BinaryArchive b;
    b.Write("x", uint32_t(42));
    const auto bd = b.GetData();

    Serializable::CborArchive c;
    c.Write("x", uint32_t(42));
    const auto cd = c.GetData();

    Mutate<Serializable::BinaryArchive>(bd);
    Mutate<Serializable::CborArchive>(cd);

    std::vector<uint8_t> truncated = {'E', 'S', 'P', 'B', 2};
    Serializable::BinaryArchive bad;
    assert(!bad.Load(truncated));

    {
        Serializable::BinaryArchiveDecodeLimits limits;
        limits.MaximumDepth = 8;
        auto data = DeepObject(16);
        Serializable::BinaryArchive archive;
        assert(!archive.Load(data.data(), data.size(), limits));
        assert(!archive.IsValid());
        assert(!Serializable::ValidateBinaryArchive(
            data.data(), data.size(), limits
        ));
    }

    {
        auto data = Header();
        data.push_back(static_cast<uint8_t>(
            Serializable::SerializationNodeType::Object
        ));
        AppendU16(data, 2);
        AppendU16(data, 1); data.push_back('a');
        data.push_back(static_cast<uint8_t>(
            Serializable::SerializationNodeType::UnsignedInteger
        ));
        for (unsigned i = 0; i < 8; ++i) data.push_back(i == 0 ? 7u : 0u);
        AppendU16(data, 1); data.push_back('b');
        data.push_back(static_cast<uint8_t>(
            Serializable::SerializationNodeType::UnsignedInteger
        ));
        for (unsigned i = 0; i < 8; ++i) data.push_back(i == 0 ? 9u : 0u);

        CountingVisitor visitor;
        assert(Serializable::VisitBinaryArchive(
            data.data(), data.size(), visitor
        ));
        assert(visitor.Properties == 2);
        assert(visitor.UnsignedValues == 2);
    }

    return 0;
}
