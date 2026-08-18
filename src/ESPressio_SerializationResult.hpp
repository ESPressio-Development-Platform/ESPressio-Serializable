#pragma once
#include <cstdint>
#include <string>
#include <vector>
namespace ESPressio::Serializable {
enum class SerializationErrorCode : uint8_t { None, MissingRequiredProperty, TypeMismatch, NumericOutOfRange, ValidationFailed, UnsupportedSchemaVersion, MigrationFailed, MalformedInput, StreamError, DuplicateValue, UnknownEnumValue };
struct SerializationIssue { SerializationErrorCode Code=SerializationErrorCode::None; std::string Path; std::string Message; };
class DeserializationResult {
    std::vector<SerializationIssue> _issues;
public:
    bool Success() const { return _issues.empty(); }
    explicit operator bool() const { return Success(); }
    const std::vector<SerializationIssue>& Issues() const { return _issues; }
    void Add(SerializationErrorCode code,const std::string& path,const std::string& message){_issues.push_back({code,path,message});}
    void Merge(const DeserializationResult& other,const std::string& prefix=""){ for(auto i:other.Issues()){ if(!prefix.empty()) i.Path=prefix+(i.Path.empty()?"":".")+i.Path; _issues.push_back(std::move(i)); } }
};
}
