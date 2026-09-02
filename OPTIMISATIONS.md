# ESPressio Serializable Optimisation Log

This file records the current resource-optimisation round chronologically. Version numbers are intentionally unchanged during this round.

Rollback anchor: `rollback/optimisations-pre-20260825` -> `39aa6be7b5d671f258f7f09aea4458c66134fd36`.

Working branch: `optimisation/25-psram-buffers`.

## 2026-08-25 — Prefer PSRAM for serialization buffers with safe fallback (#25)

### Context
Full-stack ESP32 hardware validation showed internal-capable DRAM effectively exhausted while StickA still had roughly 2 MB of PSRAM free. WiFi/ESP-NOW and several ESP-IDF facilities require internal memory, whereas normal JSON/CBOR/Binary serialization buffers are CPU-accessed storage and can safely live in external RAM on PSRAM-capable targets.

Serializable already centralizes buffer allocation through `SerializationAllocator<T>` / `SerializationBuffer<T>` and already exposed ESP32 heap-capability allocators. The existing `Esp32PreferPsramAllocator` alias, however, was identical to the PSRAM-only allocator and therefore had no internal-memory fallback.

### Changes
- `Esp32PreferPsramAllocator<T>` is now a real allocator which first attempts `MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT` and falls back to `MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT` if external allocation is unavailable or exhausted.
- explicit `Esp32InternalAllocator` and `Esp32PsramAllocator` semantics remain unchanged;
- allocation size overflow is checked before multiplying element count by element size;
- allocator equality/rebind support is provided for STL compatibility;
- defining `ESPRESSIO_SERIALIZATION_PREFER_PSRAM` on ESP32 changes the default `SerializationAllocator<T>` policy to `Esp32PreferPsramAllocator<T>`;
- without that opt-in macro, or on non-ESP32 platforms, the existing `std::allocator` default remains unchanged.

### Safety / rollout
This is deliberately opt-in during hardware validation. The Lab will enable the policy so serialization traffic can be measured without forcing a global allocator-policy change on unrelated consumers. Boards without usable PSRAM transparently fall back to internal 8-bit memory.

Commits:
- `084e353be76a203218a089821ebbaba1aa6c1948` — implement safe prefer-PSRAM allocator;
- `dfcf0651e3099495ca56d9a0f7c66d36177860a3` — add opt-in default serialization policy.

No release/version number changes were made.
