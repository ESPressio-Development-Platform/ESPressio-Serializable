#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace ESPressio::Serializable {

enum class SerializationErrorCode : uint8_t {
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

enum class ValidationBehavior : uint8_t {
    FailFast,
    CollectAll
};

struct DeserializationOptions {
    ValidationBehavior Behavior = ValidationBehavior::CollectAll;
    size_t MaximumIssues = 64;
};

struct SerializationIssue {
    SerializationErrorCode Code = SerializationErrorCode::None;
    std::string Path;
    std::string Message;
};

inline std::string JoinSerializationPath(
    const std::string& prefix,
    const std::string& suffix
) {
    if (prefix.empty()) return suffix;
    if (suffix.empty()) return prefix;
    if (!suffix.empty() && suffix.front() == '[') return prefix + suffix;
    return prefix + "." + suffix;
}

class DeserializationResult {
    std::vector<SerializationIssue> _issues;
public:
    bool Success() const { return _issues.empty(); }
    explicit operator bool() const { return Success(); }
    const std::vector<SerializationIssue>& Issues() const { return _issues; }
    size_t IssueCount() const { return _issues.size(); }

    void Add(
        SerializationErrorCode code,
        const std::string& path,
        const std::string& message,
        const DeserializationOptions& options = {}
    ) {
        if (_issues.size() >= options.MaximumIssues) return;
        _issues.push_back({code, path, message});
    }

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

    bool ShouldContinue(const DeserializationOptions& options) const {
        if (_issues.size() >= options.MaximumIssues) return false;
        return options.Behavior == ValidationBehavior::CollectAll || _issues.empty();
    }
};

}
