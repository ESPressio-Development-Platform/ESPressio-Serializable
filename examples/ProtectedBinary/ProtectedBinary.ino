#include <Arduino.h>
#include <array>
#include <ESPressio_Serializable_Security.hpp>
#include <ESPressio_Security.hpp>

using namespace ESPressio;

class WiFiLikeConfiguration final
    : public Serializable::Serializable<WiFiLikeConfiguration> {

    ESPRESSIO_SERIALIZABLE_TYPE(WiFiLikeConfiguration)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)

private:
    std::string _ssid = "ESPressio-Lab";
    std::string _password = "replace-me";
    uint8_t _channel = 6;

public:
    ESPRESSIO_SERIALIZABLE_PROPERTIES(
        ESPRESSIO_PROPERTY("ssid", _ssid),
        ESPRESSIO_PROPERTY("password", _password),
        ESPRESSIO_PROPERTY("channel", _channel)
    )

    const std::string& SSID() const { return _ssid; }
    uint8_t Channel() const { return _channel; }
};

Security::AES256GCMCipher cipher;
Security::AeadCipherRegistry ciphers;
Security::StaticKeyProvider keys;
Security::ESP32RandomSource randomSource;

// Convenience example only. Production devices that require resistance to
// firmware extraction should use an appropriately provisioned IKeyProvider.
constexpr std::array<uint8_t, 32> ApplicationKey = {
    0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
    0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,
    0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
    0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F
};

void setup() {
    Serial.begin(115200);

    ciphers.Register(cipher);
    keys.Add(1, Security::AeadAlgorithm::AES256GCM, ApplicationKey);

    Security::DataProtectionConfig dataProtection;
    dataProtection.Algorithm = Security::AeadAlgorithm::AES256GCM;
    dataProtection.KeyID = 1;

    Security::DataProtector protector(ciphers, keys, randomSource, dataProtection);
    Serializable::SerializationProtectionConfig protection(
        protector,
        "Example.WiFiLikeConfiguration"
    );

    WiFiLikeConfiguration original;
    std::vector<uint8_t> protectedBytes;

    auto encoded = Serializable::SerializeProtectedBinary(
        original,
        protectedBytes,
        protection
    );

    if (!encoded) {
        Serial.printf("protect failed: %s\n",
            Serializable::ProtectedSerializationStatusName(encoded.Status));
        return;
    }

    WiFiLikeConfiguration restored;
    auto decoded = Serializable::DeserializeProtectedBinary(
        protectedBytes.data(),
        protectedBytes.size(),
        restored,
        protection
    );

    if (!decoded) {
        Serial.printf("restore failed: %s\n",
            Serializable::ProtectedSerializationStatusName(decoded.Status));
        return;
    }

    Serial.printf("restored SSID=%s channel=%u\n",
        restored.SSID().c_str(),
        static_cast<unsigned>(restored.Channel()));
}

void loop() {}
