#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <type_traits>

namespace ESPressio::Serializable {

    /// <summary>Flags controlling how a serializable property participates in serialization and deserialization.</summary>
    enum class SerializationPropertyFlags : uint8_t {
        None=0, Required=1u<<0u, ReadOnly=1u<<1u, Sensitive=1u<<2u
    };

    constexpr SerializationPropertyFlags operator|(SerializationPropertyFlags a, SerializationPropertyFlags b) {
        return static_cast<SerializationPropertyFlags>(static_cast<uint8_t>(a)|static_cast<uint8_t>(b));
    }
    constexpr SerializationPropertyFlags operator&(SerializationPropertyFlags a, SerializationPropertyFlags b) {
        return static_cast<SerializationPropertyFlags>(static_cast<uint8_t>(a)&static_cast<uint8_t>(b));
    }
    /// <summary>Determines whether a property-flags value contains the requested flag.</summary>
    constexpr bool HasFlag(SerializationPropertyFlags value, SerializationPropertyFlags flag) {
        return (value & flag) != SerializationPropertyFlags::None;
    }

    /// <summary>Describes one serializable member and its validation, alias, default-value, and sensitivity metadata.</summary>
    /// <typeparam name="TObject">Object type owning the member.</typeparam>
    /// <typeparam name="TValue">Member value type.</typeparam>
    /// <typeparam name="TMaximumAliases">Maximum number of alternate serialized names retained by the property.</typeparam>
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

            /// <summary>Creates metadata for an object member under the supplied serialized name.</summary>
            SerializationProperty(const char* name, TValue TObject::* member)
                : _name(name), _member(member) {}

            /// <summary>Returns the primary serialized property name.</summary>
            const char* GetName() const { return _name; }
            /// <summary>Returns the pointer-to-member represented by this property.</summary>
            TValue TObject::* GetMember() const { return _member; }
            /// <summary>Returns the configured property flags.</summary>
            SerializationPropertyFlags GetFlags() const { return _flags; }
            /// <summary>Indicates whether the property must be present when deserializing.</summary>
            bool IsRequired() const { return HasFlag(_flags,SerializationPropertyFlags::Required); }
            /// <summary>Indicates whether deserialization must leave the member unchanged.</summary>
            bool IsReadOnly() const { return HasFlag(_flags,SerializationPropertyFlags::ReadOnly); }
            /// <summary>Indicates whether serialization should apply the configured sensitive-property policy.</summary>
            bool IsSensitive() const { return HasFlag(_flags,SerializationPropertyFlags::Sensitive); }
            /// <summary>Returns the number of accepted alternate property names.</summary>
            size_t GetAliasCount() const { return _aliasCount; }
            /// <summary>Returns an alternate property name, or <c>nullptr</c> when the index is out of range.</summary>
            const char* GetAlias(size_t i) const { return i<_aliasCount?_aliases[i]:nullptr; }
            /// <summary>Indicates whether a default value is configured.</summary>
            bool HasDefault() const { return _defaultValue.has_value(); }
            /// <summary>Returns the configured default value.</summary>
            const TValue& GetDefault() const { return *_defaultValue; }
            /// <summary>Indicates whether a custom validator is configured.</summary>
            bool HasValidator() const { return _validator!=nullptr; }
            /// <summary>Validates a value against numeric range constraints and the optional custom validator.</summary>
            bool ValidateValue(const TValue& value) const {
                if constexpr (std::is_arithmetic_v<TValue>) {
                    if (_minimum && value < *_minimum) return false;
                    if (_maximum && value > *_maximum) return false;
                }
                return _validator==nullptr || _validator(value);
            }

            /// <summary>Returns a copy configured as required when enabled.</summary>
            SerializationProperty Required(bool enabled=true) const { auto c=*this; if(enabled)c._flags=c._flags|SerializationPropertyFlags::Required; return c; }
            /// <summary>Returns a copy configured as read-only when enabled.</summary>
            SerializationProperty ReadOnly(bool enabled=true) const { auto c=*this; if(enabled)c._flags=c._flags|SerializationPropertyFlags::ReadOnly; return c; }
            /// <summary>Returns a copy configured as sensitive when enabled.</summary>
            SerializationProperty Sensitive(bool enabled=true) const { auto c=*this; if(enabled)c._flags=c._flags|SerializationPropertyFlags::Sensitive; return c; }
            /// <summary>Returns a copy accepting an additional serialized-name alias when capacity permits.</summary>
            SerializationProperty Alias(const char* alias) const { auto c=*this; if(alias&&c._aliasCount<TMaximumAliases)c._aliases[c._aliasCount++]=alias; return c; }
            /// <summary>Returns a copy with a default value used when the property is absent.</summary>
            SerializationProperty Default(TValue value) const { auto c=*this; c._defaultValue=std::move(value); return c; }
            /// <summary>Returns a copy using the supplied value validator.</summary>
            SerializationProperty Validate(bool (*validator)(const TValue&)) const { auto c=*this; c._validator=validator; return c; }

            /// <summary>Returns a copy constrained to the supplied inclusive arithmetic range.</summary>
            template<typename U=TValue, std::enable_if_t<std::is_arithmetic_v<U>,int> = 0>
            SerializationProperty Range(U minimum, U maximum) const { auto c=*this; c._minimum=static_cast<TValue>(minimum); c._maximum=static_cast<TValue>(maximum); return c; }

            /// <summary>Returns mutable access to the represented member of an object.</summary>
            TValue& GetValue(TObject& object) const { return object.*_member; }
            /// <summary>Returns const access to the represented member of an object.</summary>
            const TValue& GetValue(const TObject& object) const { return object.*_member; }
    };

    /// <summary>Creates serialization metadata for an object member.</summary>
    template<typename TObject, typename TValue>
    auto MakeSerializationProperty(const char* name, TValue TObject::* member) {
        return SerializationProperty<TObject,TValue>(name,member);
    }
}
