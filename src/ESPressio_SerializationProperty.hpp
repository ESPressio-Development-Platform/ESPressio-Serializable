#pragma once

#include <tuple>
#include <utility>

namespace ESPressio::Serializable {

    template<typename TObject, typename TValue>
    class SerializationProperty {
        private:
            const char* _name;
            TValue TObject::* _member;

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

            constexpr const char* GetName() const {
                return _name;
            }

            constexpr TValue TObject::* GetMember() const {
                return _member;
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
        return SerializationProperty<TObject, TValue>(
            name,
            member
        );
    }

}
