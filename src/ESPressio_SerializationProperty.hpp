#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace ESPressio::Serializable {

    enum class SerializationPropertyFlags : uint8_t {
        None      = 0,
        Required  = 1u << 0u,
        ReadOnly  = 1u << 1u,
        Sensitive = 1u << 2u
    };

    constexpr SerializationPropertyFlags operator|(
        SerializationPropertyFlags lhs,
        SerializationPropertyFlags rhs
    ) {
        return static_cast<SerializationPropertyFlags>(
            static_cast<uint8_t>(lhs) |
            static_cast<uint8_t>(rhs)
        );
    }

    constexpr SerializationPropertyFlags operator&(
        SerializationPropertyFlags lhs,
        SerializationPropertyFlags rhs
    ) {
        return static_cast<SerializationPropertyFlags>(
            static_cast<uint8_t>(lhs) &
            static_cast<uint8_t>(rhs)
        );
    }

    constexpr bool HasFlag(
        SerializationPropertyFlags value,
        SerializationPropertyFlags flag
    ) {
        return (value & flag) != SerializationPropertyFlags::None;
    }

    template<typename TObject, typename TValue, size_t TMaximumAliases = 4>
    class SerializationProperty {
        private:
            const char* _name;
            TValue TObject::* _member;
            SerializationPropertyFlags _flags =
                SerializationPropertyFlags::None;
            std::array<const char*, TMaximumAliases> _aliases {};
            size_t _aliasCount = 0;

        public:
            using ObjectType = TObject;
            using ValueType = TValue;

            constexpr SerializationProperty(
                const char* name,
                TValue TObject::* member
            )
                : _name(name),
                  _member(member) {
            }

            constexpr const char* GetName() const { return _name; }
            constexpr TValue TObject::* GetMember() const { return _member; }
            constexpr SerializationPropertyFlags GetFlags() const { return _flags; }
            constexpr bool IsRequired() const {
                return HasFlag(_flags, SerializationPropertyFlags::Required);
            }
            constexpr bool IsReadOnly() const {
                return HasFlag(_flags, SerializationPropertyFlags::ReadOnly);
            }
            constexpr bool IsSensitive() const {
                return HasFlag(_flags, SerializationPropertyFlags::Sensitive);
            }
            constexpr size_t GetAliasCount() const { return _aliasCount; }
            constexpr const char* GetAlias(size_t index) const {
                return index < _aliasCount ? _aliases[index] : nullptr;
            }

            constexpr SerializationProperty Required(bool enabled = true) const {
                auto copy = *this;
                if (enabled) {
                    copy._flags =
                        copy._flags |
                        SerializationPropertyFlags::Required;
                }
                return copy;
            }

            constexpr SerializationProperty ReadOnly(bool enabled = true) const {
                auto copy = *this;
                if (enabled) {
                    copy._flags =
                        copy._flags |
                        SerializationPropertyFlags::ReadOnly;
                }
                return copy;
            }

            constexpr SerializationProperty Sensitive(bool enabled = true) const {
                auto copy = *this;
                if (enabled) {
                    copy._flags =
                        copy._flags |
                        SerializationPropertyFlags::Sensitive;
                }
                return copy;
            }

            constexpr SerializationProperty Alias(const char* alias) const {
                auto copy = *this;
                if (
                    alias != nullptr &&
                    copy._aliasCount < TMaximumAliases
                ) {
                    copy._aliases[copy._aliasCount++] = alias;
                }
                return copy;
            }

            TValue& GetValue(TObject& object) const {
                return object.*_member;
            }

            const TValue& GetValue(const TObject& object) const {
                return object.*_member;
            }
    };

    template<typename TObject, typename TValue>
    constexpr auto MakeSerializationProperty(
        const char* name,
        TValue TObject::* member
    ) {
        return SerializationProperty<TObject, TValue>(name, member);
    }

}
