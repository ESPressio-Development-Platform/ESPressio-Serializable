#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ESPressio::Serializable {

/// <summary>Identifies the category of a serialization or deserialization failure.</summary>
/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
enum
class SerializationErrorCode : uint8_t {
    None,
    MissingRequiredProperty,
    TypeMismatch,
    NumericOutOfRange,
    ValidationFailed,
    UnsupportedSchemaVersion,
    MigrationFailed,
    MalformedInput,
    StreamError,
    DuplicateValue,
    UnknownEnumValue,
    ResourceLimitExceeded
};

/// <summary>Controls whether deserialization validation stops at the first issue or collects multiple issues.</summary>
/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
enum
class ValidationBehavior : uint8_t {
    FailFast,
    CollectAll
};

/// <summary>Configures validation behavior and issue accumulation during deserialization.</summary>
/**
 * ESPressio Memory Audit
 * Members:
 * - Behavior (ValidationBehavior): 1 bytes [0 bytes dynamic allocation]
 * - MaximumIssues (size_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 8 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * End ESPressio Memory Audit
 */
struct DeserializationOptions {
    /// <summary>Validation strategy used when an issue is encountered.</summary>
    ValidationBehavior Behavior = ValidationBehavior::CollectAll;
    /// <summary>Maximum number of issues retained in a result.</summary>
    size_t MaximumIssues = 64;
};

/// <summary>Describes one validation or decoding problem with its error code and logical property path.</summary>
/**
 * ESPressio Memory Audit
 * Members:
 * - Code (SerializationErrorCode): 1 bytes [0 bytes dynamic allocation]
 * - Path (std::string): 24 bytes [Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * - Message (std::string): 24 bytes [Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Total Memory: 52 bytes [Path: Capacity + 1 bytes when capacity exceeds 15-byte SSO; Message: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
struct SerializationIssue {
    SerializationErrorCode Code = SerializationErrorCode::None;
    std::string Path;
    std::string Message;
};

/// <summary>Combines two serialization-path components using property or array-index notation.</summary>
inline std::string JoinSerializationPath(
    const std::string& prefix,
    const std::string& suffix
) {
    if (prefix.empty()) return suffix;
    if (suffix.empty()) return prefix;
    if (!suffix.empty() && suffix.front() == '[') return prefix + suffix;
    return prefix + "." + suffix;
}

/// <summary>Accumulates validation and decoding issues produced while deserializing an object graph.</summary>
/**
 * ESPressio Memory Audit
 * Members:
 * - _issues (std::vector<SerializationIssue>): 12 bytes [Capacity * (52 bytes) element storage; N live elements each: Path: Capacity + 1 bytes when capacity exceeds 15-byte SSO; N live elements each: Message: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Total Memory: 12 bytes [_issues: Capacity * (52 bytes) element storage; _issues: N live elements each: Path: Capacity + 1 bytes when capacity exceeds 15-byte SSO; _issues: N live elements each: Message: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Basis: ESP32/Xtensa ILP32 reference ABI (4-byte pointers/size_t); ESPressio stateful allocators/deleters included; ABI-sensitive STL/platform internals are identified explicitly.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
class DeserializationResult {
    std::vector<SerializationIssue> _issues;
public:
    /// <summary>Indicates whether deserialization completed without recorded issues.</summary>
    bool Success() const { return _issues.empty(); }
    /// <summary>Converts the result to <c>true</c> when no issues were recorded.</summary>
    explicit operator bool() const { return Success(); }
    /// <summary>Returns the collected deserialization issues.</summary>
    const std::vector<SerializationIssue>& Issues() const { return _issues; }
    /// <summary>Returns the number of collected issues.</summary>
    size_t IssueCount() const { return _issues.size(); }

    /// <summary>Adds an issue unless the configured issue limit has already been reached.</summary>
    void Add(
        SerializationErrorCode code,
        const std::string& path,
        const std::string& message,
        const DeserializationOptions& options = {}
    ) {
        if (_issues.size() >= options.MaximumIssues) return;
        _issues.push_back({code, path, message});
    }

    /// <summary>Merges another result, optionally prefixing each imported issue path.</summary>
    void Merge(
        const DeserializationResult& other,
        const std::string& prefix = "",
        const DeserializationOptions& options = {}
    ) {
        for (auto issue : other.Issues()) {
            if (_issues.size() >= options.MaximumIssues) break;
            issue.Path = JoinSerializationPath(prefix, issue.Path);
            _issues.push_back(std::move(issue));
            if (options.Behavior == ValidationBehavior::FailFast) break;
        }
    }

    /// <summary>Indicates whether validation should continue under the supplied options.</summary>
    bool ShouldContinue(const DeserializationOptions& options) const {
        if (_issues.size() >= options.MaximumIssues) return false;
        return options.Behavior == ValidationBehavior::CollectAll || _issues.empty();
    }
};

}
