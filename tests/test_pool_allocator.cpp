#include <ESPressio_Memory.hpp>
#include <ESPressio_Serializable.hpp>
#include <ESPressio_CborArchive.hpp>

#include <cassert>
#include <cstddef>
#include <new>
#include <tuple>

using namespace ESPressio;

class TrackingProvider final : public System::Memory::IMemoryProvider {
public:
    std::size_t ExternalPreferredAllocations = 0;

    void* Allocate(
        std::size_t bytes,
        std::size_t alignment,
        System::Memory::MemoryPolicy policy
    ) override {
        if (policy == System::Memory::MemoryPolicy::ExternalPreferred) {
            ++ExternalPreferredAllocations;
        }
        return System::Memory::DefaultProvider().Allocate(bytes, alignment, policy);
    }

    void Deallocate(
        void* pointer,
        std::size_t bytes,
        std::size_t alignment,
        System::Memory::MemoryPolicy policy
    ) noexcept override {
        System::Memory::DefaultProvider().Deallocate(pointer, bytes, alignment, policy);
    }

    bool Supports(System::Memory::MemoryPolicy policy) const noexcept override {
        return System::Memory::DefaultProvider().Supports(policy);
    }
};

class P : public Serializable::Serializable<P> {
    ESPRESSIO_SERIALIZABLE_TYPE(P)
private:
    int _x = 1;
public:
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY("x", _x)
    )
};

int main() {
    static TrackingProvider provider;
    auto* previous = System::Memory::SetProvider(&provider);

    static_assert(
        std::tuple_size<decltype(P::GetSerializableProperties())>::value == 1,
        "serializable property metadata must remain tuple-compatible"
    );

    const std::size_t beforeProperties =
        provider.ExternalPreferredAllocations;
    const auto firstProperties = P::GetSerializableProperties();
    const std::size_t afterFirstProperties =
        provider.ExternalPreferredAllocations;

    assert(afterFirstProperties > beforeProperties);

    const auto secondProperties = P::GetSerializableProperties();
    assert(provider.ExternalPreferredAllocations == afterFirstProperties);
    assert(
        &std::get<0>(firstProperties) ==
        &std::get<0>(secondProperties)
    );

    {
        P p;
        Serializable::CborArchive archive;
        p.Serialize(archive);
        auto data = archive.GetData();
        assert(!data.empty());
        assert(provider.ExternalPreferredAllocations > 0);
    }

    System::Memory::SetProvider(previous);
    return 0;
}