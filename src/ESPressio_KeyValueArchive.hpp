#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <type_traits>
#include <vector>

namespace ESPressio::Serializable {

    /// <summary>Minimal string-backed reference archive used by examples and host tests.</summary>
    /// <remarks>This archive demonstrates the generic Serializable Read/Write contract and is intentionally independent of JSON, CBOR, NVS, or other production wire/storage formats.</remarks>
    class KeyValueArchive {
        public:
            /// <summary>One stored name/value pair.</summary>
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
            /// <summary>Returns all currently stored entries.</summary>
            const std::vector<Entry>& GetEntries() const {
                return _entries;
            }

            /// <summary>Removes all entries from the archive.</summary>
            void Clear() {
                _entries.clear();
            }

            /// <summary>Writes a string value under a name.</summary>
            void Write(const char* name, const std::string& value) {
                Set(name, value);
            }

            /// <summary>Writes a C-string value under a name; a null pointer is stored as an empty string.</summary>
            void Write(const char* name, const char* value) {
                Set(name, value == nullptr ? "" : value);
            }

            /// <summary>Writes a Boolean value using its textual representation.</summary>
            void Write(const char* name, bool value) {
                Set(name, value ? "true" : "false");
            }

            /// <summary>Writes a non-Boolean integral value using decimal text.</summary>
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

            /// <summary>Writes a floating-point value using textual conversion.</summary>
            template<
                typename TValue,
                std::enable_if_t<std::is_floating_point_v<TValue>, int> = 0
            >
            void Write(const char* name, TValue value) {
                Set(name, std::to_string(
                    static_cast<long double>(value)
                ));
            }

            /// <summary>Reads a stored string value.</summary>
            bool Read(const char* name, std::string& value) const {
                const Entry* entry = Find(name);

                if (entry == nullptr) {
                    return false;
                }

                value = entry->Value;
                return true;
            }

            /// <summary>Reads a stored Boolean value from <c>true</c> or <c>1</c> textual representations.</summary>
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

            /// <summary>Reads a signed integral value from decimal text.</summary>
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

            /// <summary>Reads an unsigned integral value from decimal text.</summary>
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

            /// <summary>Reads a floating-point value from text.</summary>
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
