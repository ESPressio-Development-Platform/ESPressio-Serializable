#include <cassert>
#include <cstdint>
#include <random>
#include <vector>

#include <ESPressio_BinaryArchive.hpp>
#include <ESPressio_CborArchive.hpp>

using namespace ESPressio;

template<class A>
void Mutate(const std::vector<uint8_t>& seed) {
    std::mt19937 rng(0x45535052);
    for (int n = 0; n < 2000; ++n) {
        auto d = seed;
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

    // Regression for #2: recursive payloads must be rejected before they can
    // consume the task stack or build an unbounded intermediate tree.
    {
        Serializable::BinaryArchiveDecodeLimits limits;
        limits.MaximumDepth = 8;
        auto data = DeepObject(16);
        Serializable::BinaryArchive archive;
        assert(!archive.Load(data.data(), data.size(), limits));
        assert(!archive.IsValid());
    }

    // Reject collection counts before attempting to construct their children.
    {
        auto data = Header();
        data.push_back(static_cast<uint8_t>(
            Serializable::SerializationNodeType::Object
        ));
        AppendU16(data, 64);

        Serializable::BinaryArchiveDecodeLimits limits;
        limits.MaximumObjectMembers = 8;
        Serializable::BinaryArchive archive;
        assert(!archive.Load(data.data(), data.size(), limits));
    }

    {
        auto data = Header();
        data.push_back(static_cast<uint8_t>(
            Serializable::SerializationNodeType::Array
        ));
        AppendU32(data, 1024);

        Serializable::BinaryArchiveDecodeLimits limits;
        limits.MaximumArrayElements = 16;
        Serializable::BinaryArchive archive;
        assert(!archive.Load(data.data(), data.size(), limits));
    }

    // Reject oversized names and values before allocating/copying them.
    {
        auto data = Header();
        data.push_back(static_cast<uint8_t>(
            Serializable::SerializationNodeType::Object
        ));
        AppendU16(data, 1);
        AppendU16(data, 32);
        data.insert(data.end(), 32, 'n');
        data.push_back(static_cast<uint8_t>(
            Serializable::SerializationNodeType::Null
        ));

        Serializable::BinaryArchiveDecodeLimits limits;
        limits.MaximumNameLength = 8;
        Serializable::BinaryArchive archive;
        assert(!archive.Load(data.data(), data.size(), limits));
    }

    {
        auto data = Header();
        data.push_back(static_cast<uint8_t>(
            Serializable::SerializationNodeType::Object
        ));
        AppendU16(data, 1);
        AppendU16(data, 1);
        data.push_back('s');
        data.push_back(static_cast<uint8_t>(
            Serializable::SerializationNodeType::String
        ));
        AppendU32(data, 128);
        data.insert(data.end(), 128, 'x');

        Serializable::BinaryArchiveDecodeLimits limits;
        limits.MaximumStringLength = 16;
        Serializable::BinaryArchive archive;
        assert(!archive.Load(data.data(), data.size(), limits));
    }

    // Aggregate node budget catches broad-but-individually-valid trees.
    {
        auto data = Header();
        data.push_back(static_cast<uint8_t>(
            Serializable::SerializationNodeType::Object
        ));
        AppendU16(data, 4);
        for (int i = 0; i < 4; ++i) {
            AppendU16(data, 1);
            data.push_back(static_cast<uint8_t>('a' + i));
            data.push_back(static_cast<uint8_t>(
                Serializable::SerializationNodeType::Null
            ));
        }

        Serializable::BinaryArchiveDecodeLimits limits;
        limits.MaximumTotalNodes = 3;
        Serializable::BinaryArchive archive;
        assert(!archive.Load(data.data(), data.size(), limits));
    }

    // Normal data remains accepted under default limits.
    {
        Serializable::BinaryArchive archive;
        assert(archive.Load(bd));
        uint32_t value = 0;
        assert(archive.Read("x", value));
        assert(value == 42);
    }

    return 0;
}
