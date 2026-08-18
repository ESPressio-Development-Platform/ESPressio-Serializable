#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>

namespace ESPressio::Serializable {

    enum class SerializationPropertyFlags : uint8_t {
        None=0, Required=1u<<0u, ReadOnly=1u<<1u, Sensitive=1u<<2u
    };

    constexpr SerializationPropertyFlags operator|(SerializationPropertyFlags a, SerializationPropertyFlags b) {
        return static_cast<SerializationPropertyFlags>(static_cast<uint8_t>(a)|static_cast<uint8_t>(b));
    }
    constexpr SerializationPropertyFlags operator&(SerializationPropertyFlags a, SerializationPropertyFlags b) {
        return static_cast<SerializationPropertyFlags>(static_cast<uint8_t>(a)&static_cast<uint8_t>(b));
    }
    constexpr bool HasFlag(SerializationPropertyFlags value, SerializationPropertyFlags flag) {
        return (value & flag) != SerializationPropertyFlags::None;
    }

    template<typename TObject, typename TValue, size_t TMaximumAliases=4>
    class SerializationProperty {
        private:
            const char* _name;
            TValue TObject::* _member;
            SerializationPropertyFlags _flags=SerializationPropertyFlags::None;
            std::array<const char*, TMaximumAliases> _aliases{};
            size_t _aliasCount=0;
            std::optional<TValue> _defaultValue;
            bool (*_validator)(const TValue&)=nullptr;
            std::optional<TValue> _minimum;
            std::optional<TValue> _maximum;

        public:
            using ObjectType=TObject;
            using ValueType=TValue;

            SerializationProperty(const char* name, TValue TObject::* member)
                : _name(name), _member(member) {}

            const char* GetName() const { return _name; }
            TValue TObject::* GetMember() const { return _member; }
            SerializationPropertyFlags GetFlags() const { return _flags; }
            bool IsRequired() const { return HasFlag(_flags,SerializationPropertyFlags::Required); }
            bool IsReadOnly() const { return HasFlag(_flags,SerializationPropertyFlags::ReadOnly); }
            bool IsSensitive() const { return HasFlag(_flags,SerializationPropertyFlags::Sensitive); }
            size_t GetAliasCount() const { return _aliasCount; }
            const char* GetAlias(size_t i) const { return i<_aliasCount?_aliases[i]:nullptr; }
            bool HasDefault() const { return _defaultValue.has_value(); }
            const TValue& GetDefault() const { return *_defaultValue; }
            bool HasValidator() const { return _validator!=nullptr; }
            bool ValidateValue(const TValue& value) const {
                if constexpr (std::is_arithmetic_v<TValue>) {
                    if (_minimum && value < *_minimum) return false;
                    if (_maximum && value > *_maximum) return false;
                }
                return _validator==nullptr || _validator(value);
            }

            SerializationProperty Required(bool enabled=true) const { auto c=*this; if(enabled)c._flags=c._flags|SerializationPropertyFlags::Required; return c; }
            SerializationProperty ReadOnly(bool enabled=true) const { auto c=*this; if(enabled)c._flags=c._flags|SerializationPropertyFlags::ReadOnly; return c; }
            SerializationProperty Sensitive(bool enabled=true) const { auto c=*this; if(enabled)c._flags=c._flags|SerializationPropertyFlags::Sensitive; return c; }
            SerializationProperty Alias(const char* alias) const { auto c=*this; if(alias&&c._aliasCount<TMaximumAliases)c._aliases[c._aliasCount++]=alias; return c; }
            SerializationProperty Default(TValue value) const { auto c=*this; c._defaultValue=std::move(value); return c; }
            SerializationProperty Validate(bool (*validator)(const TValue&)) const { auto c=*this; c._validator=validator; return c; }

            template<typename U=TValue, std::enable_if_t<std::is_arithmetic_v<U>,int> = 0>
            SerializationProperty Range(U minimum, U maximum) const { auto c=*this; c._minimum=static_cast<TValue>(minimum); c._maximum=static_cast<TValue>(maximum); return c; }

            TValue& GetValue(TObject& object) const { return object.*_member; }
            const TValue& GetValue(const TObject& object) const { return object.*_member; }
    };

    template<typename TObject, typename TValue>
    auto MakeSerializationProperty(const char* name, TValue TObject::* member) {
        return SerializationProperty<TObject,TValue>(name,member);
    }
}
