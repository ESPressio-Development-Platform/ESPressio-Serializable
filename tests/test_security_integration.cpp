#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

#include "ESPressio_Serializable_Security.hpp"

using namespace ESPressio;

class TestConfiguration final : public Serializable::Serializable<TestConfiguration> {
    ESPRESSIO_SERIALIZABLE_TYPE(TestConfiguration)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(2)
private:
    uint32_t _sampleRate = 0;
    std::string _name;
    bool _enabled = false;
public:
    TestConfiguration() = default;
    TestConfiguration(uint32_t sampleRate, std::string name, bool enabled)
        : _sampleRate(sampleRate), _name(std::move(name)), _enabled(enabled) {}
    uint32_t SampleRate() const { return _sampleRate; }
    const std::string& Name() const { return _name; }
    bool Enabled() const { return _enabled; }
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY("sampleRate", _sampleRate),
        ESPRESSIO_PROPERTY("name", _name),
        ESPRESSIO_PROPERTY("enabled", _enabled)
    )
};

class TestProtector final : public Security::IDataProtector {
public:
    Security::SecurityResult Protect(const uint8_t* plaintext, std::size_t plaintextSize,
        std::vector<uint8_t>& output, const Security::DataProtectionContext& context = {}) override {
        if (plaintext == nullptr && plaintextSize != 0) return Security::SecurityResult::Fail(Security::SecurityError::InvalidArgument, "bad input");
        output.clear(); output.push_back(ContextByte(context));
        for (std::size_t i=0;i<plaintextSize;++i) output.push_back(static_cast<uint8_t>(plaintext[i] ^ 0xA5u));
        return Security::SecurityResult::Ok(true);
    }
    Security::SecurityResult Unprotect(const uint8_t* input, std::size_t inputSize,
        std::vector<uint8_t>& plaintext, const Security::DataProtectionContext& context = {}) override {
        plaintext.clear();
        if (input == nullptr || inputSize == 0 || input[0] != ContextByte(context))
            return Security::SecurityResult::Fail(Security::SecurityError::AuthenticationFailed, "context mismatch");
        for (std::size_t i=1;i<inputSize;++i) plaintext.push_back(static_cast<uint8_t>(input[i] ^ 0xA5u));
        return Security::SecurityResult::Ok(true);
    }
private:
    static uint8_t ContextByte(const Security::DataProtectionContext& context) {
        uint8_t value = 0; for (std::size_t i=0;i<context.Size;++i) value = static_cast<uint8_t>((value * 33u) ^ context.Data[i]); return value;
    }
};

int main() {
    TestProtector protector;
    Serializable::SerializationProtectionConfig protection(protector, "test.configuration");

    TestConfiguration source(48000, "recorder", true);
    std::vector<uint8_t> protectedBytes;
    auto saved = Serializable::SerializeProtectedBinary(source, protectedBytes, protection);
    assert(saved.Success());
    assert(saved.ArchiveBytes > 0 && saved.ProtectedBytes == protectedBytes.size());

    TestConfiguration restored;
    auto loaded = Serializable::DeserializeProtectedBinary(protectedBytes.data(), protectedBytes.size(), restored, protection);
    assert(loaded.Success());
    assert(restored.SampleRate() == 48000);
    assert(restored.Name() == "recorder");
    assert(restored.Enabled());

    Serializable::SerializationProtectionConfig wrongContext(protector, "other.configuration");
    TestConfiguration rejected;
    auto rejectedResult = Serializable::DeserializeProtectedBinary(protectedBytes.data(), protectedBytes.size(), rejected, wrongContext);
    assert(!rejectedResult.Success());
    assert(rejectedResult.Status == Serializable::ProtectedSerializationStatus::UnprotectionFailed);
    assert(rejectedResult.SecurityResult.Error == Security::SecurityError::AuthenticationFailed);

    // The configuration-driven API must also preserve the ordinary unprotected ESPB path.
    std::vector<uint8_t> ordinary;
    assert(Serializable::SerializeBinary(source, ordinary, nullptr).Success());
    TestConfiguration ordinaryRestored;
    assert(Serializable::DeserializeBinary(ordinary.data(), ordinary.size(), ordinaryRestored, nullptr).Success());
    assert(ordinaryRestored.Name() == "recorder");

    protection.MaximumArchiveBytes = 1;
    std::vector<uint8_t> limited;
    assert(Serializable::SerializeProtectedBinary(source, limited, protection).Status == Serializable::ProtectedSerializationStatus::PayloadTooLarge);

    return 0;
}
