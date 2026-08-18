#pragma once

#if !__has_include(<Arduino.h>)
    #error "ESPressio_StreamArchive.hpp requires the Arduino framework because it operates on Arduino Stream."
#endif

#include <Arduino.h>
#include <cstdint>
#include <vector>

namespace ESPressio::Serializable {

    /*
     * Stream is a transport, not a serialization representation.
     *
     * StreamArchive transports the byte payload produced by an archive
     * implementing GetData()/Load(). A 32-bit little-endian payload length is
     * prepended so that multiple serialized objects can safely share a stream.
     *
     * It is directly usable with BinaryArchive and CborArchive.
     */
    class StreamArchive {
        private:
            static bool ReadExact(
                Stream& stream,
                uint8_t* output,
                size_t size,
                uint32_t timeoutMilliseconds
            ) {
                const uint32_t started = millis();
                size_t received = 0;

                while (received < size) {
                    while (
                        received < size &&
                        stream.available() > 0
                    ) {
                        const int value = stream.read();

                        if (value >= 0) {
                            output[received++] =
                                static_cast<uint8_t>(value);
                        }
                    }

                    if (
                        static_cast<uint32_t>(
                            millis() - started
                        ) >= timeoutMilliseconds
                    ) {
                        return false;
                    }

                    yield();
                }

                return true;
            }

        public:
            template<typename TArchive, typename TObject>
            static bool Serialize(
                const TObject& object,
                Stream& stream
            ) {
                TArchive archive;
                object.Serialize(archive);

                const std::vector<uint8_t> data =
                    archive.GetData();

                const uint32_t size =
                    static_cast<uint32_t>(data.size());

                uint8_t header[4] = {
                    static_cast<uint8_t>(size & 0xFFu),
                    static_cast<uint8_t>((size >> 8u) & 0xFFu),
                    static_cast<uint8_t>((size >> 16u) & 0xFFu),
                    static_cast<uint8_t>((size >> 24u) & 0xFFu)
                };

                if (stream.write(header, sizeof(header)) != sizeof(header)) {
                    return false;
                }

                return
                    data.empty() ||
                    stream.write(
                        data.data(),
                        data.size()
                    ) == data.size();
            }

            template<typename TArchive, typename TObject>
            static bool Deserialize(
                Stream& stream,
                TObject& object,
                uint32_t timeoutMilliseconds = 1000
            ) {
                uint8_t header[4];

                if (
                    !ReadExact(
                        stream,
                        header,
                        sizeof(header),
                        timeoutMilliseconds
                    )
                ) {
                    return false;
                }

                const uint32_t size =
                    static_cast<uint32_t>(header[0]) |
                    (static_cast<uint32_t>(header[1]) << 8u) |
                    (static_cast<uint32_t>(header[2]) << 16u) |
                    (static_cast<uint32_t>(header[3]) << 24u);

                std::vector<uint8_t> data(size);

                if (
                    size > 0 &&
                    !ReadExact(
                        stream,
                        data.data(),
                        data.size(),
                        timeoutMilliseconds
                    )
                ) {
                    return false;
                }

                TArchive archive;

                if (!archive.Load(data)) {
                    return false;
                }

                object.Deserialize(archive);
                return true;
            }
    };

}
