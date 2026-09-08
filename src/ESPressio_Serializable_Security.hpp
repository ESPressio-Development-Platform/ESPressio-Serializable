#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>

#include "ESPressio_Serializable_Binary.hpp"
#include <ESPressio_IDataProtector.hpp>

namespace ESPressio::Serializable {

/**
 * ESPressio Memory Audit
 * Underlying storage: 1 bytes
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
enum
/**
 * ESPressio Memory Audit
 * Inherited Memory Total: 1 bytes [0 bytes dynamic allocation]
 * Members: none (empty object still occupies at least 1 byte unless empty-base optimisation applies).
 * Total Memory: 1 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
class ProtectedSerializationStatus : uint8_t {
    Success = 0,
    InvalidArgument,
    SerializationFailed,
    ProtectionFailed,
    UnprotectionFailed,
    PayloadTooLarge,
    MalformedArchive,
    DeserializationFailed
};

inline const char* ProtectedSerializationStatusName(ProtectedSerializationStatus status) {
    switch (status) {
        case ProtectedSerializationStatus::Success: return "Success";
        case ProtectedSerializationStatus::InvalidArgument: return "InvalidArgument";
        case ProtectedSerializationStatus::SerializationFailed: return "SerializationFailed";
        case ProtectedSerializationStatus::ProtectionFailed: return "ProtectionFailed";
        case ProtectedSerializationStatus::UnprotectionFailed: return "UnprotectionFailed";
        case ProtectedSerializationStatus::PayloadTooLarge: return "PayloadTooLarge";
        case ProtectedSerializationStatus::MalformedArchive: return "MalformedArchive";
        case ProtectedSerializationStatus::DeserializationFailed: return "DeserializationFailed";
        default: return "Unknown";
    }
}

/**
 * ESPressio Memory Audit
 * Members:
 * - Protector (Security::IDataProtector*): 4 bytes [0 bytes dynamic allocation]
 * - Context (std::string): 24 bytes [Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * - MaximumArchiveBytes (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 32 bytes [Context: Capacity + 1 bytes when capacity exceeds 15-byte SSO]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * Confidence: medium; compile-time sizeof on the concrete target remains authoritative for ABI-sensitive/opaque members.
 * End ESPressio Memory Audit
 */
struct SerializationProtectionConfig {
    Security::IDataProtector* Protector = nullptr;
    std::string Context;
    std::size_t MaximumArchiveBytes = 64u * 1024u;
    BinaryArchiveDecodeLimits DecodeLimits{};
    DeserializationOptions Deserialization{};

    SerializationProtectionConfig() = default;

    explicit SerializationProtectionConfig(
        Security::IDataProtector& protector,
        std::string context = {}
    ) : Protector(&protector), Context(std::move(context)) {}

    Security::DataProtectionContext GetContext() const {
        return Security::DataProtectionContext(Context);
    }
};

/**
 * ESPressio Memory Audit
 * Members:
 * - Status (ProtectedSerializationStatus): 1 bytes [0 bytes dynamic allocation]
 * - ArchiveBytes (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * - ProtectedBytes (std::size_t): 4 bytes [0 bytes dynamic allocation]
 * Total Memory: 12 bytes [0 bytes dynamic allocation]
 * Basis: ESP32/Xtensa ILP32 reference ABI; GNU libstdc++ container control-block sizes are implementation-sensitive.
 * End ESPressio Memory Audit
 */
struct ProtectedSerializationResult {
    ProtectedSerializationStatus Status = ProtectedSerializationStatus::Success;
    std::size_t ArchiveBytes = 0;
    std::size_t ProtectedBytes = 0;
    Security::SecurityResult SecurityResult = Security::SecurityResult::Ok(false);
    DeserializationResult Deserialization{};

    bool Success() const { return Status == ProtectedSerializationStatus::Success; }
    explicit operator bool() const { return Success(); }
};

template<typename TObject>
ProtectedSerializationResult SerializeProtectedBinary(
    const TObject& object,
    SerializationBuffer<uint8_t>& output,
    const SerializationProtectionConfig& protection
) {
    ProtectedSerializationResult result;
    output.clear();
    if (protection.Protector == nullptr || protection.MaximumArchiveBytes == 0) {
        result.Status = ProtectedSerializationStatus::InvalidArgument;
        return result;
    }

    SerializationBuffer<uint8_t> archiveBytes;
    try {
        BinaryArchive archive;
        object.Serialize(archive);
        archiveBytes = archive.GetData();
    } catch (...) {
        result.Status = ProtectedSerializationStatus::SerializationFailed;
        return result;
    }

    result.ArchiveBytes = archiveBytes.size();
    if (archiveBytes.size() > protection.MaximumArchiveBytes) {
        result.Status = ProtectedSerializationStatus::PayloadTooLarge;
        return result;
    }

    result.SecurityResult = protection.Protector->Protect(
        archiveBytes.data(), archiveBytes.size(), output, protection.GetContext()
    );
    result.ProtectedBytes = output.size();
    if (!result.SecurityResult.Success) {
        output.clear();
        result.Status = ProtectedSerializationStatus::ProtectionFailed;
    }
    return result;
}

template<typename TObject>
ProtectedSerializationResult DeserializeProtectedBinary(
    const uint8_t* protectedData,
    std::size_t protectedDataSize,
    TObject& object,
    const SerializationProtectionConfig& protection
) {
    ProtectedSerializationResult result;
    result.ProtectedBytes = protectedDataSize;
    if (protection.Protector == nullptr || protectedData == nullptr ||
        protectedDataSize == 0 || protection.MaximumArchiveBytes == 0) {
        result.Status = ProtectedSerializationStatus::InvalidArgument;
        return result;
    }

    SerializationBuffer<uint8_t> archiveBytes;
    result.SecurityResult = protection.Protector->Unprotect(
        protectedData, protectedDataSize, archiveBytes, protection.GetContext()
    );
    if (!result.SecurityResult.Success) {
        result.Status = ProtectedSerializationStatus::UnprotectionFailed;
        return result;
    }

    result.ArchiveBytes = archiveBytes.size();
    if (archiveBytes.size() > protection.MaximumArchiveBytes) {
        result.Status = ProtectedSerializationStatus::PayloadTooLarge;
        return result;
    }

    BinaryArchive archive;
    if (!archive.Load(archiveBytes.data(), archiveBytes.size(), protection.DecodeLimits)) {
        result.Status = ProtectedSerializationStatus::MalformedArchive;
        return result;
    }

    try {
        result.Deserialization = object.DeserializeDetailed(archive, protection.Deserialization);
    } catch (...) {
        result.Status = ProtectedSerializationStatus::DeserializationFailed;
        return result;
    }

    if (!result.Deserialization.Success()) {
        result.Status = ProtectedSerializationStatus::DeserializationFailed;
    }
    return result;
}

template<typename TObject>
ProtectedSerializationResult SerializeBinary(
    const TObject& object,
    SerializationBuffer<uint8_t>& output,
    const SerializationProtectionConfig* protection
) {
    if (protection != nullptr) return SerializeProtectedBinary(object, output, *protection);

    ProtectedSerializationResult result;
    try {
        BinaryArchive archive;
        object.Serialize(archive);
        output = archive.GetData();
        result.ArchiveBytes = output.size();
        result.ProtectedBytes = output.size();
        result.SecurityResult = Security::SecurityResult::Ok(false);
    } catch (...) {
        result.Status = ProtectedSerializationStatus::SerializationFailed;
    }
    return result;
}

template<typename TObject>
ProtectedSerializationResult DeserializeBinary(
    const uint8_t* data,
    std::size_t size,
    TObject& object,
    const SerializationProtectionConfig* protection
) {
    if (protection != nullptr) return DeserializeProtectedBinary(data, size, object, *protection);

    ProtectedSerializationResult result;
    result.ArchiveBytes = size;
    result.ProtectedBytes = size;
    result.SecurityResult = Security::SecurityResult::Ok(false);
    if (data == nullptr || size == 0) {
        result.Status = ProtectedSerializationStatus::InvalidArgument;
        return result;
    }
    BinaryArchive archive;
    if (!archive.Load(data, size)) {
        result.Status = ProtectedSerializationStatus::MalformedArchive;
        return result;
    }
    result.Deserialization = object.DeserializeDetailed(archive);
    if (!result.Deserialization.Success()) {
        result.Status = ProtectedSerializationStatus::DeserializationFailed;
    }
    return result;
}

} // namespace ESPressio::Serializable
