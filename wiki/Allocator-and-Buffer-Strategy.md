# Allocator and Buffer Strategy

Serializable centralizes dynamic serialization storage through allocator/buffer abstractions so representation code can choose placement policy without changing schema semantics.

## Default behaviour

On normal/non-ESP32 builds, or without a placement opt-in, the default serialization allocator follows the standard allocation path.

## ESP32 prefer-PSRAM mode

When `ESPRESSIO_SERIALIZATION_PREFER_PSRAM` is enabled on ESP32, the default allocator first attempts 8-bit PSRAM and then falls back to internal 8-bit memory.

This is intentionally a **prefer** policy rather than PSRAM-required behaviour, allowing boards without available external RAM to continue functioning.

## Explicit allocators

Internal-only and PSRAM-only allocator choices remain meaningful when an integration has a hard placement contract.

## Extension requirements

New archive/buffer types should use the central serialization allocator aliases rather than raw `std::vector`/platform allocation where doing so would bypass configured policy.

Always guard `count * sizeof(T)` allocation arithmetic against overflow and retain STL allocator compatibility for rebind/equality semantics.

## Security

Memory placement is not a confidentiality boundary. Sensitive plaintext/key material still requires appropriate lifetime and security handling regardless of DRAM versus PSRAM placement.