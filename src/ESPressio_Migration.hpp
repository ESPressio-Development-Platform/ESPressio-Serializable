#pragma once
#include <string>
#include <vector>
#include "ESPressio_SerializationNode.hpp"

namespace ESPressio::Serializable::Migration {

    inline bool Rename(
        SerializationNode& object,
        const char* oldName,
        const char* newName,
        bool overwrite = false
    ) {
        if (!overwrite && object.Find(newName) != nullptr) {
            return false;
        }

        SerializationNode value;
        if (!object.Take(oldName, value)) {
            return false;
        }
        object.Set(newName, std::move(value));
        return true;
    }

    inline bool Remove(
        SerializationNode& object,
        const char* name
    ) {
        return object.Remove(name);
    }

    inline bool Move(
        SerializationNode& source,
        const char* sourceName,
        SerializationNode& destination,
        const char* destinationName,
        bool overwrite = false
    ) {
        if (!overwrite && destination.Find(destinationName) != nullptr) {
            return false;
        }

        SerializationNode value;
        if (!source.Take(sourceName, value)) {
            return false;
        }
        destination.Set(destinationName, std::move(value));
        return true;
    }

    inline SerializationNode* ResolveObjectPath(
        SerializationNode& root,
        const char* path,
        bool create = false
    ) {
        if (path == nullptr || *path == '\0') {
            return &root;
        }

        SerializationNode* current = &root;
        std::string token;
        const char* cursor = path;

        while (true) {
            if (*cursor == '.' || *cursor == '\0') {
                if (!token.empty()) {
                    SerializationNode* next = current->Find(token.c_str());
                    if (next == nullptr && create) {
                        current->Set(
                            token.c_str(),
                            SerializationNode(SerializationNodeType::Object)
                        );
                        next = current->Find(token.c_str());
                    }
                    if (
                        next == nullptr ||
                        next->GetType() != SerializationNodeType::Object
                    ) {
                        return nullptr;
                    }
                    current = next;
                    token.clear();
                }
                if (*cursor == '\0') {
                    break;
                }
            } else {
                token.push_back(*cursor);
            }
            ++cursor;
        }
        return current;
    }
}
