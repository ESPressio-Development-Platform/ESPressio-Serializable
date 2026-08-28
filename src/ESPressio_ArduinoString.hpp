#pragma once

#if !__has_include(<Arduino.h>)
    #error "ESPressio_ArduinoString.hpp requires the Arduino framework."
#endif

#include <Arduino.h>
#include <string>
#include "ESPressio_SerializationAdapter.hpp"
#include "ESPressio_SerializationNode.hpp"

namespace ESPressio::Serializable {
    /// <summary>Serialization adapter mapping Arduino <c>String</c> values to the common string node representation.</summary>
    template<>
    struct SerializationAdapter<String> {
        static constexpr bool Supported = true;

        /// <summary>Converts an Arduino <c>String</c> to a string serialization node.</summary>
        static SerializationNode ToNode(const String& value) {
            SerializationNode node(SerializationNodeType::String);
            node.StringValue() =
                std::string(value.c_str(), value.length());
            return node;
        }

        /// <summary>Attempts to populate an Arduino <c>String</c> from a string serialization node.</summary>
        static bool FromNode(
            const SerializationNode& node,
            String& value
        ) {
            if (node.GetType() != SerializationNodeType::String) {
                return false;
            }

            value = String(node.StringValue().c_str());
            return true;
        }
    };
}
