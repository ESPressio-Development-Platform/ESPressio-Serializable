#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <type_traits>
#include <vector>

namespace ESPressio::Serializable {

    /*
     * Tiny reference archive used by the examples and host tests.
     *
     * This is deliberately NOT a JSON implementation. The Serializable core
     * must not depend on ArduinoJson (or any other wire/storage format).
     * A production JSON/CBOR/NVS archive can implement the same Read/Write
     * surface independently.
     */
    class KeyValueArchive {
        public:
            struct Entry {
                std::string Name;
                std::string Value;
            };

        private:
            std::vector<Entry> _entries;

            const Entry* Find(const char* name) const {
                for (const auto& entry : _entries) {
                    if (entry.Name == name) {
                        return &entry;
                    }
                }

                return nullptr;
            }

            Entry* Find(const char* name) {
                for (auto& entry : _entries) {
                    if (entry.Name == name) {
                        return &entry;
                    }
                }

                return nullptr;
            }

            void Set(const char* name, const std::string& value) {
                Entry* existing = Find(name);

                if (existing != nullptr) {
                    existing->Value = value;
                    return;
                }

                _entries.push_back({name, value});
            }

        public:
            const std::vector<Entry>& GetEntries() const {
                return _entries;
            }

            void Clear() {
                _entries.clear();
            }

            void Write(const char* name, const std::string& value) {
                Set(name, value);
            }

            void Write(const char* name, const char* value) {
                Set(name, value == nullptr ? "" : value);
            }

            void Write(const char* name, bool value) {
                Set(name, value ? "true" : "false");
            }

            template<
                typename TValue,
                std::enable_if_t<
                    std::is_integral_v<TValue> &&
                    !std::is_same_v<TValue, bool>,
                    int
                > = 0
            >
            void Write(const char* name, TValue value) {
                if constexpr (std::is_signed_v<TValue>) {
                    Set(name, std::to_string(
                        static_cast<long long>(value)
                    ));
                } else {
                    Set(name, std::to_string(
                        static_cast<unsigned long long>(value)
                    ));
                }
            }

            template<
                typename TValue,
                std::enable_if_t<std::is_floating_point_v<TValue>, int> = 0
            >
            void Write(const char* name, TValue value) {
                Set(name, std::to_string(
                    static_cast<long double>(value)
                ));
            }

            bool Read(const char* name, std::string& value) const {
                const Entry* entry = Find(name);

                if (entry == nullptr) {
                    return false;
                }

                value = entry->Value;
                return true;
            }

            bool Read(const char* name, bool& value) const {
                const Entry* entry = Find(name);

                if (entry == nullptr) {
                    return false;
                }

                value =
                    entry->Value == "true" ||
                    entry->Value == "1";

                return true;
            }

            template<
                typename TValue,
                std::enable_if_t<
                    std::is_integral_v<TValue> &&
                    std::is_signed_v<TValue> &&
                    !std::is_same_v<TValue, bool>,
                    int
                > = 0
            >
            bool Read(const char* name, TValue& value) const {
                const Entry* entry = Find(name);

                if (entry == nullptr) {
                    return false;
                }

                value = static_cast<TValue>(
                    std::strtoll(entry->Value.c_str(), nullptr, 10)
                );

                return true;
            }

            template<
                typename TValue,
                std::enable_if_t<
                    std::is_integral_v<TValue> &&
                    std::is_unsigned_v<TValue> &&
                    !std::is_same_v<TValue, bool>,
                    int
                > = 0
            >
            bool Read(const char* name, TValue& value) const {
                const Entry* entry = Find(name);

                if (entry == nullptr) {
                    return false;
                }

                value = static_cast<TValue>(
                    std::strtoull(entry->Value.c_str(), nullptr, 10)
                );

                return true;
            }

            template<
                typename TValue,
                std::enable_if_t<std::is_floating_point_v<TValue>, int> = 0
            >
            bool Read(const char* name, TValue& value) const {
                const Entry* entry = Find(name);

                if (entry == nullptr) {
                    return false;
                }

                value = static_cast<TValue>(
                    std::strtold(entry->Value.c_str(), nullptr)
                );

                return true;
            }
    };

}
