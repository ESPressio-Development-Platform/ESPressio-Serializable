# ESP-NOW Serializable Mesh Example

Flash the same `ESPNowSerializableMesh.ino` sketch onto two or more ESP32 devices.

Each device periodically creates a `DeviceState`, serializes it directly as CBOR using `CborStreamSerializer`, fragments that byte stream into ESP-NOW broadcast packets, and reassembles/deserializes messages received from other ESP32s.

The example intentionally distinguishes two identities:

* **transport sender** — the source MAC reported by ESP-NOW;
* **object origin** — the MAC embedded in every ESPressio framing header.

For this direct-broadcast demo these must be identical. Keeping `OriginMac` in the framing makes it possible to extend the protocol later to forwarding/relaying while retaining the MAC of the ESP32 that originally generated the Serializable object.

## Important points

* All ESP32s must use the same Wi-Fi channel (`ESPNOW_WIFI_CHANNEL`).
* The demo keeps complete ESP-NOW packets at 220 bytes, below the ESP-NOW v1 250-byte payload maximum.
* The ESP-NOW receive callback performs no deserialization. It only copies packets into a FreeRTOS queue; reassembly and deserialization happen in `loop()`.
* The sender waits for each ESP-NOW send-completion callback before emitting another fragment.
* CBOR is used because ESPressio provides both direct streaming serialization and streaming deserialization without requiring ArduinoJson.
* The example uses broadcast, so it is suitable for two or more devices without hard-coding peer MAC addresses.

## PlatformIO

For a project that consumes the published library, the application would normally only require ESPressio Serializable:

```ini
lib_deps =
    flowduino/ESPressio-Serializable
```

ESP-NOW itself is supplied by the ESP32 Arduino core.
