#pragma once
#include <cstdint>

namespace ESPressio::Serializable {
    /// <summary>Controls how properties marked as sensitive are represented during serialization.</summary>
/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
enum
class SensitivePropertyPolicy : uint8_t {
        Include,
        Redact,
        Omit
    };

    /// <summary>Configures serialization-time handling of sensitive properties.</summary>
/**
 * ESPressio Memory Audit
 * Members:
 * - _sensitive (SensitivePropertyPolicy): 1 bytes [0 bytes dynamic allocation]
 * - _redactionText (char*): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 8 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class SerializationPolicy {
        private:
            SensitivePropertyPolicy _sensitive =
                SensitivePropertyPolicy::Include;
            const char* _redactionText = "[REDACTED]";

        public:
            /// <summary>Sets the handling policy applied to sensitive properties.</summary>
            SerializationPolicy& Sensitive(
                SensitivePropertyPolicy policy
            ) {
                _sensitive = policy;
                return *this;
            }

            /// <summary>Sets the text emitted when sensitive values are redacted.</summary>
            SerializationPolicy& RedactionText(const char* value) {
                _redactionText = value == nullptr ? "" : value;
                return *this;
            }

            /// <summary>Returns the configured sensitive-property handling policy.</summary>
            SensitivePropertyPolicy GetSensitivePolicy() const {
                return _sensitive;
            }

            /// <summary>Returns the configured sensitive-value redaction text.</summary>
            const char* GetRedactionText() const {
                return _redactionText;
            }
    };
}
