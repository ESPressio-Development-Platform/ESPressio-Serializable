# Memory and Decode Limits

Serializable is designed for embedded use where both allocation placement and decode growth must remain explicit.

## Binary decode limits

`BinaryArchiveDecodeLimits` constrains structural growth including nesting depth, total nodes, object members, array elements, property-name lengths, and string lengths.

Apply limits even to authenticated data. Authentication establishes integrity/authenticity under the configured key; it does not make an extremely large valid representation affordable to decode.

## Serialization buffers

The 1.0.0 baseline centralizes serialization buffer allocation through `SerializationAllocator<T>` / `SerializationBuffer<T>`.

On ESP32, defining:

```cpp
ESPRESSIO_SERIALIZATION_PREFER_PSRAM
```

selects the prefer-PSRAM allocator for default serialization buffers. It first attempts 8-bit PSRAM and falls back to internal 8-bit memory when external allocation is unavailable or exhausted.

Without that opt-in macro, and on non-ESP32 platforms, the normal default allocator remains unchanged.

## Explicit allocator choices

Explicit internal-only and PSRAM-only allocator semantics remain available where an integration has a hard placement requirement.

## Why serialization is a good PSRAM candidate

Normal JSON/CBOR/Binary work buffers are CPU-accessed storage and do not generally require the internal-capable DRAM needed by some WiFi, ESP-NOW, DMA, or SDK facilities. Prefer-PSRAM can therefore protect scarce internal memory without changing serialization semantics.

## Safety

Allocation size multiplication is overflow-checked, and the prefer-PSRAM mode is a placement policy—not permission to accept unbounded input.