#pragma once

#if !__has_include(<Arduino.h>) || !__has_include(<Preferences.h>)
    #error "ESPressio_NvsArchive.hpp requires Arduino ESP32 Preferences/NVS support."
#endif

#include <Arduino.h>
#include <Preferences.h>
#include <vector>
#include "ESPressio_BinaryArchive.hpp"

namespace ESPressio::Serializable {

/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 100 bytes [BinaryArchive: TreeArchive: _root: _stringValue: _value: Capacity + 1 bytes when capacity exceeds 15-byte SSO; BinaryArchive: TreeArchive: _root: _objectChildren: Capacity * (aligned pair of (24 bytes) + (sizeof(SerializationNode) (target/toolchain dependent))) element storage; BinaryArchive: TreeArchive: _root: _objectChildren: N live elements each: _value: Capacity + 1 bytes when capacity exceeds 15-byte SSO; BinaryArchive: TreeArchive: _root: _arrayChildren: Capacity * (sizeof(SerializationNode) (target/toolchain dependent)) element storage]
 * Members: none (standalone empty object occupies 1 byte; an eligible empty base may be optimized to 0 bytes).
 * Total Memory: 100 bytes [BinaryArchive: TreeArchive: _root: _stringValue: _value: Capacity + 1 bytes when capacity exceeds 15-byte SSO; BinaryArchive: TreeArchive: _root: _objectChildren: Capacity * (aligned pair of (24 bytes) + (sizeof(SerializationNode) (target/toolchain dependent))) element storage; BinaryArchive: TreeArchive: _root: _objectChildren: N live elements each: _value: Capacity + 1 bytes when capacity exceeds 15-byte SSO; BinaryArchive: TreeArchive: _root: _arrayChildren: Capacity * (sizeof(SerializationNode) (target/toolchain dependent)) element storage]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
class NvsArchive : public BinaryArchive {
        public:
            bool Save(
                Preferences& preferences,
                const char* key
            ) const {
                const auto data = GetData();

                if (data.empty()) {
                    return false;
                }

                return preferences.putBytes(
                    key,
                    data.data(),
                    data.size()
                ) == data.size();
            }

            bool Load(
                Preferences& preferences,
                const char* key
            ) {
                const size_t size =
                    preferences.getBytesLength(key);

                if (size == 0) {
                    return false;
                }

                std::vector<uint8_t> data(size);

                if (
                    preferences.getBytes(
                        key,
                        data.data(),
                        data.size()
                    ) != data.size()
                ) {
                    return false;
                }

                return BinaryArchive::Load(data);
            }

            bool Save(
                const char* nameSpace,
                const char* key
            ) const {
                Preferences preferences;

                if (!preferences.begin(nameSpace, false)) {
                    return false;
                }

                const bool result = Save(preferences, key);
                preferences.end();
                return result;
            }

            bool Load(
                const char* nameSpace,
                const char* key
            ) {
                Preferences preferences;

                if (!preferences.begin(nameSpace, true)) {
                    return false;
                }

                const bool result = Load(preferences, key);
                preferences.end();
                return result;
            }
    };

}
