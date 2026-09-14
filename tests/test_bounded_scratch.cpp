#include <array>
#include <cstddef>
#include <cstdint>

#include "ESPressio_BoundedContainers.hpp"
#include "ESPressio_BoundedScratchDeserializer.hpp"
#include "ESPressio_SerializableBase.hpp"
#include "ESPressio_SerializationMacros.hpp"

using namespace ESPressio::Serializable;

namespace {

struct NonCopyBounded final : Serializable<NonCopyBounded> {
    BoundedBytes<64U> Payload{};
    std::uint32_t Counter{0U};

    NonCopyBounded() = default;
    NonCopyBounded(const NonCopyBounded&) = delete;
    NonCopyBounded& operator=(const NonCopyBounded&) = delete;
    NonCopyBounded(NonCopyBounded&&) noexcept = default;
    NonCopyBounded& operator=(NonCopyBounded&&) noexcept = default;

    ESPRESSIO_SERIALIZABLE_TYPE(NonCopyBounded)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY_REQUIRED("payload", Payload),
        ESPRESSIO_PROPERTY_REQUIRED("counter", Counter))
};

static_assert(IsBoundedSerializable<NonCopyBounded>);
static_assert(!std::is_copy_constructible_v<NonCopyBounded>);

} // namespace

int main() {
    NonCopyBounded source;
    source.Counter = 0xAABBCCDDU;
    for (std::uint8_t value = 1U; value <= 32U; ++value) {
        if (!source.Payload.push_back(value)) return 1;
    }

    std::array<std::uint8_t, MaximumSerializedSize<NonCopyBounded, DirectBinary>> bytes{};
    const auto encoded = SerializeDirectBinary(source, bytes.data(), bytes.size());
    if (!encoded) return 2;

    NonCopyBounded scratch;
    const auto decoded = DeserializeBoundedDirectBinaryIntoScratch(bytes.data(), encoded.Bytes, scratch);
    if (!decoded || decoded.Bytes != encoded.Bytes) return 3;
    if (scratch.Counter != source.Counter || scratch.Payload.size() != source.Payload.size()) return 4;
    for (std::size_t i = 0U; i < scratch.Payload.size(); ++i) {
        if (scratch.Payload[i] != source.Payload[i]) return 5;
    }

    if (DeserializeBoundedDirectBinaryIntoScratch(
            bytes.data(), MaximumSerializedSize<NonCopyBounded, DirectBinary> + 1U, scratch).Error !=
        SerializationErrorCode::ResourceLimitExceeded) return 6;

    auto malformed = bytes;
    malformed[4] = 0xFFU;
    if (DeserializeBoundedDirectBinaryIntoScratch(malformed.data(), encoded.Bytes, scratch).Error !=
        SerializationErrorCode::MalformedInput) return 7;

    return 0;
}
