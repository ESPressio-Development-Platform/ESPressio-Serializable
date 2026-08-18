#pragma once
#include <cstdint>

namespace ESPressio::Serializable {
    enum class SensitivePropertyPolicy : uint8_t {
        Include,
        Redact,
        Omit
    };

    class SerializationPolicy {
        private:
            SensitivePropertyPolicy _sensitive =
                SensitivePropertyPolicy::Include;
            const char* _redactionText = "[REDACTED]";

        public:
            SerializationPolicy& Sensitive(
                SensitivePropertyPolicy policy
            ) {
                _sensitive = policy;
                return *this;
            }

            SerializationPolicy& RedactionText(const char* value) {
                _redactionText = value == nullptr ? "" : value;
                return *this;
            }

            SensitivePropertyPolicy GetSensitivePolicy() const {
                return _sensitive;
            }

            const char* GetRedactionText() const {
                return _redactionText;
            }
    };
}
