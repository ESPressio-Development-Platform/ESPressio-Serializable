#pragma once
#include <string>
#include <vector>
#include "ESPressio_SerializationNode.hpp"

namespace ESPressio::Serializable::Migration {

    /// <summary>Renames a property within an object node by moving its value to a new name.</summary>
    /// <returns><c>true</c> when the source exists and the destination is available or overwrite is enabled.</returns>
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

    /// <summary>Removes a named property from an object node.</summary>
    inline bool Remove(
        SerializationNode& object,
        const char* name
    ) {
        return object.Remove(name);
    }

    /// <summary>Moves a named property between object nodes.</summary>
    /// <returns><c>true</c> when the source exists and the destination accepts the value.</returns>
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

    /// <summary>Resolves a dot-delimited object path, optionally creating missing object nodes.</summary>
    /// <returns>The resolved object node, or <c>nullptr</c> when the path cannot be followed.</returns>
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

    /// <summary>Resolves a property/array path such as <c>object.items[0]</c>, optionally creating missing object segments.</summary>
    inline SerializationNode* ResolvePath(
        SerializationNode& root,
        const char* path,
        bool create = false
    ) {
        if (path == nullptr || *path == '\0') {
            return &root;
        }

        SerializationNode* current = &root;
        const char* cursor = path;
        while (*cursor) {
            if (*cursor == '.') {
                ++cursor;
                continue;
            }

            if (*cursor == '[') {
                ++cursor;
                std::size_t index = 0;
                if (*cursor < '0' || *cursor > '9') {
                    return nullptr;
                }
                while (*cursor >= '0' && *cursor <= '9') {
                    index = index * 10 + static_cast<std::size_t>(*cursor - '0');
                    ++cursor;
                }
                if (*cursor != ']') {
                    return nullptr;
                }
                ++cursor;
                if (current->GetType() != SerializationNodeType::Array) {
                    return nullptr;
                }
                if (index >= current->ArrayChildren().size()) {
                    return nullptr;
                }
                current = &current->ArrayChildren()[index];
                continue;
            }

            std::string token;
            while (*cursor && *cursor != '.' && *cursor != '[') {
                token.push_back(*cursor++);
            }

            if (current->GetType() != SerializationNodeType::Object) {
                return nullptr;
            }

            auto* next = current->Find(token.c_str());
            if (next == nullptr && create) {
                current->Set(
                    token.c_str(),
                    SerializationNode(SerializationNodeType::Object)
                );
                next = current->Find(token.c_str());
            }
            if (next == nullptr) {
                return nullptr;
            }
            current = next;
        }
        return current;
    }

    /// <summary>Removes the value addressed by a property/array path.</summary>
    inline bool RemoveAt(
        SerializationNode& root,
        const char* path
    ) {
        if (path == nullptr) {
            return false;
        }

        std::string pathText(path);
        const auto dot = pathText.find_last_of('.');
        const auto bracket = pathText.find_last_of('[');
        if (
            bracket != std::string::npos &&
            (dot == std::string::npos || bracket > dot)
        ) {
            const auto close = pathText.find(']', bracket);
            if (close == std::string::npos || close != pathText.size() - 1) {
                return false;
            }

            const auto index = static_cast<std::size_t>(
                std::stoul(pathText.substr(bracket + 1, close - bracket - 1))
            );
            const std::string parent = pathText.substr(0, bracket);
            auto* node = ResolvePath(root, parent.c_str());
            if (
                node == nullptr ||
                node->GetType() != SerializationNodeType::Array ||
                index >= node->ArrayChildren().size()
            ) {
                return false;
            }

            node->ArrayChildren().erase(node->ArrayChildren().begin() + index);
            return true;
        }

        const std::string parent = dot == std::string::npos
            ? ""
            : pathText.substr(0, dot);
        const std::string name = dot == std::string::npos
            ? pathText
            : pathText.substr(dot + 1);
        auto* node = ResolvePath(root, parent.c_str());
        return node != nullptr && node->Remove(name.c_str());
    }

    /// <summary>Moves a value from an arbitrary source path into a destination object under a new name.</summary>
    inline bool MovePath(
        SerializationNode& root,
        const char* sourcePath,
        const char* destinationObjectPath,
        const char* destinationName,
        bool overwrite = false
    ) {
        auto* source = ResolvePath(root, sourcePath);
        if (source == nullptr) {
            return false;
        }

        SerializationNode copy = *source;
        auto* destination = ResolvePath(root, destinationObjectPath, true);
        if (
            destination == nullptr ||
            destination->GetType() != SerializationNodeType::Object
        ) {
            return false;
        }
        if (!overwrite && destination->Find(destinationName) != nullptr) {
            return false;
        }

        destination->Set(destinationName, std::move(copy));
        return RemoveAt(root, sourcePath);
    }

}
