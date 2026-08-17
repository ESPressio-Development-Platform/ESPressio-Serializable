#pragma once

#include <tuple>
#include <utility>

namespace ESPressio::Serializable {

    /*
     * CRTP base for compile-time serializable types.
     *
     * TArchive is intentionally duck-typed: an archive only needs to provide
     * Write(name, value) and/or Read(name, value). This keeps the core
     * representation-neutral and dependency-free.
     */
    template<typename TDerived>
    class SerializableBase {
        protected:
            constexpr SerializableBase() = default;
            ~SerializableBase() = default;

        public:
            template<typename TArchive>
            void Serialize(TArchive& archive) const {
                const TDerived& object =
                    static_cast<const TDerived&>(*this);

                std::apply(
                    [&](const auto&... property) {
                        (
                            archive.Write(
                                property.GetName(),
                                property.GetValue(object)
                            ),
                            ...
                        );
                    },
                    TDerived::GetSerializableProperties()
                );
            }

            template<typename TArchive>
            void Deserialize(TArchive& archive) {
                TDerived& object =
                    static_cast<TDerived&>(*this);

                std::apply(
                    [&](const auto&... property) {
                        (
                            archive.Read(
                                property.GetName(),
                                property.GetValue(object)
                            ),
                            ...
                        );
                    },
                    TDerived::GetSerializableProperties()
                );
            }
    };

    // Short public name for consumers.
    template<typename TDerived>
    using Serializable = SerializableBase<TDerived>;

}
