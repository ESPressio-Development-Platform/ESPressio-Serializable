# Transport integration

Serialization formats and transports remain deliberately separate. Any Arduino `Print` can receive direct JSON/CBOR/Binary streaming output; any compatible `Stream` can feed the streaming readers.

The repository includes ESP-NOW, Serial and TCP-oriented examples. `Crc32FramedStream` additionally provides reusable `ESPF` length+CRC32 framing for transports that preserve bytes but not message boundaries. It is not itself a serializer: frame an encoded CBOR/Binary payload, then decode it with the corresponding archive/stream reader.
