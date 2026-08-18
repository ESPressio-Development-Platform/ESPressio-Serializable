/*
 * ESPressio Serializable - ESP-NOW Serializable Mesh Demo
 *
 * Run the SAME sketch on two or more ESP32 devices.
 *
 * Every device:
 *   - operates on the same Wi-Fi channel;
 *   - periodically creates a local DeviceState object;
 *   - serializes it directly as CBOR;
 *   - fragments the CBOR byte stream into ESP-NOW broadcast frames;
 *   - receives broadcasts from any other device;
 *   - identifies the originating device by its Station MAC address;
 *   - reassembles the CBOR stream outside the Wi-Fi callback;
 *   - deserializes it back into DeviceState.
 *
 * This example deliberately keeps ESP-NOW frames <= 220 bytes so that the
 * framing remains comfortably inside the 250-byte ESP-NOW v1 payload limit.
 *
 * Requires:
 *   - ESP32 Arduino core with the current ESP-NOW API (`ESP32_NOW.h`)
 *   - ESPressio Serializable
 *
 * No ArduinoJson dependency is required: CBOR is used for the wire format.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ESP32_NOW.h>
#include <esp_mac.h>

#include <array>
#include <vector>

#include <ESPressio_Serializable_Arduino.hpp>
#include <ESPressio_Serializable_CBORStream.hpp>

using namespace ESPressio;

/* -------------------------------------------------------------------------- */
/* Configuration                                                              */
/* -------------------------------------------------------------------------- */

static constexpr uint8_t ESPNOW_WIFI_CHANNEL = 6;

/*
 * 220 bytes total keeps each packet below the ESP-NOW v1 250-byte payload
 * ceiling after including our own framing header.
 */
static constexpr size_t ESPNOW_FRAME_SIZE = 220;

static constexpr uint32_t FRAME_MAGIC = 0x45535053;  // "ESPS"
static constexpr uint8_t FRAME_VERSION = 1;

static constexpr uint32_t SEND_TIMEOUT_MS = 1000;
static constexpr uint32_t REASSEMBLY_TIMEOUT_MS = 5000;
static constexpr uint32_t BROADCAST_INTERVAL_MS = 5000;

static constexpr size_t MAX_REMOTE_PEERS = 16;
static constexpr size_t RX_QUEUE_DEPTH = 24;

/* -------------------------------------------------------------------------- */
/* Serializable application object                                            */
/* -------------------------------------------------------------------------- */

enum class DeviceMode : uint8_t {
    Idle,
    Active,
    Fault
};

ESPRESSIO_ENUM_MAPPING(
    DeviceMode,
    ESPRESSIO_ENUM_VALUE(DeviceMode::Idle, "idle"),
    ESPRESSIO_ENUM_VALUE(DeviceMode::Active, "active"),
    ESPRESSIO_ENUM_VALUE(DeviceMode::Fault, "fault")
)

class DeviceState final
    : public Serializable::Serializable<DeviceState> {

    ESPRESSIO_SERIALIZABLE_TYPE(DeviceState)
    ESPRESSIO_SERIALIZABLE_SCHEMA_VERSION(1)

    private:
        uint32_t _sequence = 0;
        uint32_t _uptimeMilliseconds = 0;
        float _temperatureCelsius = 0.0f;
        DeviceMode _mode = DeviceMode::Idle;
        String _name;

    public:
        DeviceState() = default;

        ESPRESSIO_SERIALIZABLE_PROPERTIES(
            ESPRESSIO_PROPERTY("sequence", _sequence),
            ESPRESSIO_PROPERTY("uptimeMs", _uptimeMilliseconds),
            ESPRESSIO_PROPERTY("temperatureC", _temperatureCelsius),
            ESPRESSIO_PROPERTY("mode", _mode),
            ESPRESSIO_PROPERTY("name", _name)
        )

        void SetSequence(uint32_t value) {
            _sequence = value;
        }

        void SetUptimeMilliseconds(uint32_t value) {
            _uptimeMilliseconds = value;
        }

        void SetTemperatureCelsius(float value) {
            _temperatureCelsius = value;
        }

        void SetMode(DeviceMode value) {
            _mode = value;
        }

        void SetName(const String& value) {
            _name = value;
        }

        uint32_t GetSequence() const {
            return _sequence;
        }

        uint32_t GetUptimeMilliseconds() const {
            return _uptimeMilliseconds;
        }

        float GetTemperatureCelsius() const {
            return _temperatureCelsius;
        }

        DeviceMode GetMode() const {
            return _mode;
        }

        const String& GetName() const {
            return _name;
        }
};

/* -------------------------------------------------------------------------- */
/* ESP-NOW framing                                                            */
/* -------------------------------------------------------------------------- */

enum class FrameType : uint8_t {
    Start = 1,
    Data = 2,
    End = 3
};

#pragma pack(push, 1)
struct EspNowFrameHeader {
    uint32_t Magic;
    uint8_t Version;
    FrameType Type;

    /*
     * MAC address of the ESP32 on which the Serializable object originated.
     *
     * This is intentionally part of the framing even though ESP-NOW itself
     * exposes the source MAC. It allows forwarded/relayed payloads to retain
     * their original source identity in a future extension of the example.
     */
    uint8_t OriginMac[6];

    uint32_t MessageId;
    uint16_t FragmentIndex;
    uint16_t PayloadLength;
};
#pragma pack(pop)

static constexpr size_t FRAME_HEADER_SIZE =
    sizeof(EspNowFrameHeader);

static constexpr size_t FRAME_PAYLOAD_SIZE =
    ESPNOW_FRAME_SIZE - FRAME_HEADER_SIZE;

struct ReceivedFrame {
    uint8_t SourceMac[6];
    size_t Length;
    uint8_t Data[ESPNOW_FRAME_SIZE];
};

static QueueHandle_t receiveQueue = nullptr;

/* -------------------------------------------------------------------------- */
/* MAC helpers                                                                */
/* -------------------------------------------------------------------------- */

static std::array<uint8_t, 6> localMac {};

static bool MacEquals(
    const uint8_t* left,
    const uint8_t* right
) {
    return memcmp(left, right, 6) == 0;
}

static String MacToString(const uint8_t* mac) {
    char buffer[18];

    snprintf(
        buffer,
        sizeof(buffer),
        "%02X:%02X:%02X:%02X:%02X:%02X",
        mac[0],
        mac[1],
        mac[2],
        mac[3],
        mac[4],
        mac[5]
    );

    return String(buffer);
}

/* -------------------------------------------------------------------------- */
/* Receive callback hand-off                                                  */
/* -------------------------------------------------------------------------- */

/*
 * ESP-NOW callbacks execute on the Wi-Fi task.
 *
 * Do not deserialize here. We only copy the received frame into a FreeRTOS
 * queue and perform all parsing/reassembly from loop().
 */
static void QueueReceivedFrame(
    const uint8_t* sourceMac,
    const uint8_t* data,
    size_t length
) {
    if (
        receiveQueue == nullptr ||
        sourceMac == nullptr ||
        data == nullptr ||
        length == 0 ||
        length > ESPNOW_FRAME_SIZE
    ) {
        return;
    }

    ReceivedFrame frame {};
    memcpy(frame.SourceMac, sourceMac, 6);
    frame.Length = length;
    memcpy(frame.Data, data, length);

    xQueueSend(
        receiveQueue,
        &frame,
        0
    );
}

/* -------------------------------------------------------------------------- */
/* ESP-NOW peers                                                              */
/* -------------------------------------------------------------------------- */

class RemotePeer final : public ESP_NOW_Peer {
    public:
        explicit RemotePeer(const uint8_t* mac)
            : ESP_NOW_Peer(
                mac,
                ESPNOW_WIFI_CHANNEL,
                WIFI_IF_STA,
                nullptr
            ) {
        }

        void onReceive(
            const uint8_t* data,
            size_t length,
            bool broadcast
        ) override {
            (void)broadcast;

            QueueReceivedFrame(
                addr(),
                data,
                static_cast<size_t>(length)
            );
        }

        void onSent(bool success) override {
            (void)success;
        }
};

class BroadcastPeer final : public ESP_NOW_Peer {
    private:
        volatile bool _sendCompleted = false;
        volatile bool _sendSuccessful = false;

    public:
        BroadcastPeer()
            : ESP_NOW_Peer(
                ESP_NOW.BROADCAST_ADDR,
                ESPNOW_WIFI_CHANNEL,
                WIFI_IF_STA,
                nullptr
            ) {
        }

        void onReceive(
            const uint8_t* data,
            size_t length,
            bool broadcast
        ) override {
            (void)data;
            (void)length;
            (void)broadcast;
        }

        void onSent(bool success) override {
            _sendSuccessful = success;
            _sendCompleted = true;
        }

        bool SendAndWait(
            const uint8_t* data,
            size_t length
        ) {
            _sendCompleted = false;
            _sendSuccessful = false;

            if (
                send(
                    data,
                    static_cast<int>(length)
                ) != length
            ) {
                return false;
            }

            const uint32_t started = millis();

            while (!_sendCompleted) {
                if (
                    static_cast<uint32_t>(
                        millis() - started
                    ) >= SEND_TIMEOUT_MS
                ) {
                    return false;
                }

                delay(1);
            }

            return _sendSuccessful;
        }
};

static BroadcastPeer broadcastPeer;
static std::vector<RemotePeer*> remotePeers;

static RemotePeer* FindRemotePeer(
    const uint8_t* mac
) {
    for (auto* peer : remotePeers) {
        if (
            peer != nullptr &&
            MacEquals(peer->addr(), mac)
        ) {
            return peer;
        }
    }

    return nullptr;
}

/*
 * Called for the first packet received from an unknown peer.
 *
 * We must enqueue THIS packet as well, otherwise the first fragment of a newly
 * discovered sender would be lost before the RemotePeer is registered.
 */
static void OnNewPeer(
    const esp_now_recv_info_t* info,
    const uint8_t* data,
    int length,
    void* argument
) {
    (void)argument;

    if (
        info == nullptr ||
        data == nullptr ||
        length <= 0
    ) {
        return;
    }

    QueueReceivedFrame(
        info->src_addr,
        data,
        static_cast<size_t>(length)
    );

    if (
        FindRemotePeer(info->src_addr) != nullptr ||
        remotePeers.size() >= MAX_REMOTE_PEERS
    ) {
        return;
    }

    auto* peer =
        new RemotePeer(info->src_addr);

    if (!peer->add()) {
        delete peer;
        return;
    }

    remotePeers.push_back(peer);
}

/* -------------------------------------------------------------------------- */
/* CBOR -> ESP-NOW streaming writer                                           */
/* -------------------------------------------------------------------------- */

/*
 * CborStreamSerializer writes to Arduino Print.
 *
 * This Print implementation buffers only one ESP-NOW fragment at a time.
 * When the fragment fills, it is immediately broadcast and the writer waits
 * for ESP-NOW's send callback before continuing. This intentionally follows
 * Espressif's guidance not to fire many ESP-NOW sends back-to-back without
 * allowing the previous send callback to complete.
 */
class EspNowMessageWriter final : public Print {
    private:
        BroadcastPeer& _peer;
        uint8_t _payload[FRAME_PAYLOAD_SIZE] {};
        size_t _payloadLength = 0;

        uint32_t _messageId = 0;
        uint16_t _fragmentIndex = 0;
        bool _failed = false;

        bool SendFrame(
            FrameType type,
            const uint8_t* payload,
            size_t payloadLength
        ) {
            if (
                payloadLength > FRAME_PAYLOAD_SIZE
            ) {
                return false;
            }

            uint8_t frame[ESPNOW_FRAME_SIZE] {};

            auto* header =
                reinterpret_cast<EspNowFrameHeader*>(
                    frame
                );

            header->Magic = FRAME_MAGIC;
            header->Version = FRAME_VERSION;
            header->Type = type;
            memcpy(
                header->OriginMac,
                localMac.data(),
                localMac.size()
            );

            header->MessageId = _messageId;
            header->FragmentIndex = _fragmentIndex;
            header->PayloadLength =
                static_cast<uint16_t>(
                    payloadLength
                );

            if (
                payloadLength > 0 &&
                payload != nullptr
            ) {
                memcpy(
                    frame + FRAME_HEADER_SIZE,
                    payload,
                    payloadLength
                );
            }

            const bool result =
                _peer.SendAndWait(
                    frame,
                    FRAME_HEADER_SIZE +
                        payloadLength
                );

            if (type == FrameType::Data) {
                ++_fragmentIndex;
            }

            return result;
        }

        bool FlushPayload() {
            if (_payloadLength == 0) {
                return true;
            }

            const bool result =
                SendFrame(
                    FrameType::Data,
                    _payload,
                    _payloadLength
                );

            _payloadLength = 0;

            if (!result) {
                _failed = true;
            }

            return result;
        }

    public:
        explicit EspNowMessageWriter(
            BroadcastPeer& peer
        )
            : _peer(peer) {
        }

        bool Begin(uint32_t messageId) {
            _messageId = messageId;
            _fragmentIndex = 0;
            _payloadLength = 0;
            _failed = false;

            return SendFrame(
                FrameType::Start,
                nullptr,
                0
            );
        }

        bool End() {
            if (!FlushPayload()) {
                return false;
            }

            if (
                !SendFrame(
                    FrameType::End,
                    nullptr,
                    0
                )
            ) {
                _failed = true;
            }

            return !_failed;
        }

        size_t write(uint8_t value) override {
            return write(&value, 1);
        }

        size_t write(
            const uint8_t* buffer,
            size_t size
        ) override {
            if (
                buffer == nullptr ||
                size == 0 ||
                _failed
            ) {
                return 0;
            }

            size_t consumed = 0;

            while (consumed < size) {
                const size_t available =
                    FRAME_PAYLOAD_SIZE -
                    _payloadLength;

                const size_t amount =
                    min(
                        available,
                        size - consumed
                    );

                memcpy(
                    _payload + _payloadLength,
                    buffer + consumed,
                    amount
                );

                _payloadLength += amount;
                consumed += amount;

                if (
                    _payloadLength ==
                    FRAME_PAYLOAD_SIZE
                ) {
                    if (!FlushPayload()) {
                        return consumed;
                    }
                }
            }

            return consumed;
        }

        bool Failed() const {
            return _failed;
        }
};

/* -------------------------------------------------------------------------- */
/* Reassembly                                                                 */
/* -------------------------------------------------------------------------- */

struct ReassemblySlot {
    bool Active = false;
    uint8_t OriginMac[6] {};
    uint32_t MessageId = 0;
    uint16_t NextFragmentIndex = 0;
    uint32_t LastActivity = 0;
    std::vector<uint8_t> Payload;
};

static std::vector<ReassemblySlot> reassemblySlots;

static ReassemblySlot* FindReassemblySlot(
    const uint8_t* originMac,
    uint32_t messageId
) {
    for (auto& slot : reassemblySlots) {
        if (
            slot.Active &&
            slot.MessageId == messageId &&
            MacEquals(
                slot.OriginMac,
                originMac
            )
        ) {
            return &slot;
        }
    }

    return nullptr;
}

static ReassemblySlot& StartReassembly(
    const EspNowFrameHeader& header
) {
    for (auto& slot : reassemblySlots) {
        if (!slot.Active) {
            slot.Active = true;
            memcpy(
                slot.OriginMac,
                header.OriginMac,
                6
            );

            slot.MessageId =
                header.MessageId;

            slot.NextFragmentIndex = 0;
            slot.LastActivity = millis();
            slot.Payload.clear();

            return slot;
        }
    }

    reassemblySlots.emplace_back();

    auto& slot =
        reassemblySlots.back();

    slot.Active = true;
    memcpy(
        slot.OriginMac,
        header.OriginMac,
        6
    );

    slot.MessageId =
        header.MessageId;

    slot.NextFragmentIndex = 0;
    slot.LastActivity = millis();

    return slot;
}

/* -------------------------------------------------------------------------- */
/* Read-only Stream over a reassembled byte vector                            */
/* -------------------------------------------------------------------------- */

class VectorInputStream final : public Stream {
    private:
        const std::vector<uint8_t>& _data;
        size_t _position = 0;

    public:
        explicit VectorInputStream(
            const std::vector<uint8_t>& data
        )
            : _data(data) {
        }

        int available() override {
            return static_cast<int>(
                _data.size() - _position
            );
        }

        int read() override {
            if (_position >= _data.size()) {
                return -1;
            }

            return _data[_position++];
        }

        int peek() override {
            if (_position >= _data.size()) {
                return -1;
            }

            return _data[_position];
        }

        void flush() override {
        }

        size_t write(uint8_t) override {
            return 0;
        }
};

/* -------------------------------------------------------------------------- */
/* Received object handling                                                   */
/* -------------------------------------------------------------------------- */

static void PrintDeviceState(
    const DeviceState& state,
    const uint8_t* originMac,
    bool local
) {
    Serial.printf(
        "\n[%s] DeviceState from %s\n",
        local ? "LOCAL" : "REMOTE",
        MacToString(originMac).c_str()
    );

    Serial.printf(
        "  name:        %s\n",
        state.GetName().c_str()
    );

    Serial.printf(
        "  sequence:    %lu\n",
        static_cast<unsigned long>(
            state.GetSequence()
        )
    );

    Serial.printf(
        "  uptime:      %lu ms\n",
        static_cast<unsigned long>(
            state.GetUptimeMilliseconds()
        )
    );

    Serial.printf(
        "  temperature: %.2f C\n",
        state.GetTemperatureCelsius()
    );
}

static void ConsumeCompletedMessage(
    ReassemblySlot& slot
) {
    /*
     * The explicit origin MAC carried inside the framing determines whether
     * the object originated locally or remotely.
     *
     * In normal broadcast operation, our own ESP-NOW broadcast is not treated
     * as a received packet, so this should normally be REMOTE. Keeping this
     * comparison makes the object-source distinction explicit and also makes
     * future relaying/loopback demonstrations straightforward.
     */
    const bool local =
        MacEquals(
            slot.OriginMac,
            localMac.data()
        );

    VectorInputStream input(
        slot.Payload
    );

    Serializable::CborStreamDeserializer
        deserializer(input);

    DeviceState state;

    const auto result =
        deserializer.Deserialize(state);

    if (!result) {
        Serial.printf(
            "Failed to deserialize CBOR message %lu from %s\n",
            static_cast<unsigned long>(
                slot.MessageId
            ),
            MacToString(
                slot.OriginMac
            ).c_str()
        );

        for (const auto& issue : result.Issues()) {
            Serial.printf(
                "  path='%s': %s\n",
                issue.Path.c_str(),
                issue.Message.c_str()
            );
        }

        return;
    }

    PrintDeviceState(
        state,
        slot.OriginMac,
        local
    );
}

static void ProcessReceivedFrame(
    const ReceivedFrame& received
) {
    if (
        received.Length <
        FRAME_HEADER_SIZE
    ) {
        return;
    }

    const auto* header =
        reinterpret_cast<
            const EspNowFrameHeader*
        >(received.Data);

    if (
        header->Magic != FRAME_MAGIC ||
        header->Version != FRAME_VERSION ||
        header->PayloadLength >
            FRAME_PAYLOAD_SIZE ||
        FRAME_HEADER_SIZE +
            header->PayloadLength >
            received.Length
    ) {
        return;
    }

    /*
     * For this demo the object should originate on the radio sender itself.
     * Reject a forged/mismatched origin MAC.
     *
     * If you later extend the design to relay Serializable objects through
     * intermediate ESP32s, remove this equality requirement while retaining
     * OriginMac as the original source identity.
     */
    if (
        !MacEquals(
            header->OriginMac,
            received.SourceMac
        )
    ) {
        Serial.printf(
            "Rejected frame: transport sender %s claims origin %s\n",
            MacToString(
                received.SourceMac
            ).c_str(),
            MacToString(
                header->OriginMac
            ).c_str()
        );

        return;
    }

    switch (header->Type) {
        case FrameType::Start: {
            auto* existing =
                FindReassemblySlot(
                    header->OriginMac,
                    header->MessageId
                );

            if (existing != nullptr) {
                existing->Active = false;
                existing->Payload.clear();
            }

            StartReassembly(*header);
            break;
        }

        case FrameType::Data: {
            auto* slot =
                FindReassemblySlot(
                    header->OriginMac,
                    header->MessageId
                );

            if (slot == nullptr) {
                return;
            }

            /*
             * The sender waits for ESP-NOW completion between fragments, so
             * fragments should arrive in sequence. If one is missing/out of
             * order, discard the incomplete object rather than deserialize
             * corrupted state.
             */
            if (
                header->FragmentIndex !=
                slot->NextFragmentIndex
            ) {
                slot->Active = false;
                slot->Payload.clear();
                return;
            }

            const uint8_t* payload =
                received.Data +
                FRAME_HEADER_SIZE;

            slot->Payload.insert(
                slot->Payload.end(),
                payload,
                payload +
                    header->PayloadLength
            );

            ++slot->NextFragmentIndex;
            slot->LastActivity = millis();
            break;
        }

        case FrameType::End: {
            auto* slot =
                FindReassemblySlot(
                    header->OriginMac,
                    header->MessageId
                );

            if (slot == nullptr) {
                return;
            }

            ConsumeCompletedMessage(*slot);

            slot->Active = false;
            slot->Payload.clear();
            break;
        }
    }
}

static void ProcessReceiveQueue() {
    ReceivedFrame frame {};

    while (
        xQueueReceive(
            receiveQueue,
            &frame,
            0
        ) == pdTRUE
    ) {
        ProcessReceivedFrame(frame);
    }
}

static void ExpireIncompleteMessages() {
    const uint32_t now = millis();

    for (auto& slot : reassemblySlots) {
        if (
            slot.Active &&
            static_cast<uint32_t>(
                now - slot.LastActivity
            ) >= REASSEMBLY_TIMEOUT_MS
        ) {
            slot.Active = false;
            slot.Payload.clear();
        }
    }
}

/* -------------------------------------------------------------------------- */
/* Local object generation / broadcast                                        */
/* -------------------------------------------------------------------------- */

static uint32_t objectSequence = 0;
static uint32_t messageId = 0;

static DeviceState CreateLocalState() {
    DeviceState state;

    state.SetSequence(
        ++objectSequence
    );

    state.SetUptimeMilliseconds(
        millis()
    );

    /*
     * Synthetic value purely for demonstration.
     * Replace this with a real sensor reading.
     */
    state.SetTemperatureCelsius(
        20.0f +
        static_cast<float>(
            objectSequence % 100
        ) /
        10.0f
    );

    state.SetMode(
        DeviceMode::Active
    );

    state.SetName(
        String("ESP32-") +
        MacToString(
            localMac.data()
        ).substring(12)
    );

    return state;
}

static bool BroadcastState(
    const DeviceState& state
) {
    EspNowMessageWriter writer(
        broadcastPeer
    );

    const uint32_t currentMessageId =
        ++messageId;

    if (!writer.Begin(currentMessageId)) {
        return false;
    }

    /*
     * The Serializable object is traversed directly into CBOR bytes. The
     * EspNowMessageWriter receives those bytes via the normal Arduino Print
     * abstraction and fragments them into ESP-NOW packets.
     */
    Serializable::CborStreamSerializer
        serializer(writer);

    serializer.Serialize(state);

    return writer.End();
}

/* -------------------------------------------------------------------------- */
/* Setup                                                                      */
/* -------------------------------------------------------------------------- */

static bool BeginEspNow() {
    WiFi.mode(WIFI_STA);

    WiFi.setChannel(
        ESPNOW_WIFI_CHANNEL,
        WIFI_SECOND_CHAN_NONE
    );

    while (!WiFi.STA.started()) {
        delay(10);
    }

    esp_read_mac(
        localMac.data(),
        ESP_MAC_WIFI_STA
    );

    if (!ESP_NOW.begin()) {
        return false;
    }

    /*
     * A broadcast peer must be registered before broadcast transmission.
     */
    if (!broadcastPeer.add()) {
        return false;
    }

    /*
     * Unknown transmitters are discovered dynamically. Once registered as
     * RemotePeer objects their subsequent packets are delivered to onReceive.
     */
    ESP_NOW.onNewPeer(
        OnNewPeer,
        nullptr
    );

    return true;
}

void setup() {
    Serial.begin(115200);
    delay(250);

    receiveQueue =
        xQueueCreate(
            RX_QUEUE_DEPTH,
            sizeof(ReceivedFrame)
        );

    if (receiveQueue == nullptr) {
        Serial.println(
            "Failed to create receive queue."
        );

        while (true) {
            delay(1000);
        }
    }

    if (!BeginEspNow()) {
        Serial.println(
            "Failed to initialize ESP-NOW."
        );

        while (true) {
            delay(1000);
        }
    }

    Serial.println();
    Serial.println(
        "ESPressio Serializable ESP-NOW Mesh Demo"
    );

    Serial.printf(
        "Local MAC: %s\n",
        MacToString(
            localMac.data()
        ).c_str()
    );

    Serial.printf(
        "Wi-Fi channel: %u\n",
        ESPNOW_WIFI_CHANNEL
    );

    Serial.printf(
        "ESP-NOW version: %d, max payload: %d bytes\n",
        ESP_NOW.getVersion(),
        ESP_NOW.getMaxDataLen()
    );

    Serial.println(
        "Flash this same sketch onto two or more ESP32 devices."
    );
}

/* -------------------------------------------------------------------------- */
/* Loop                                                                       */
/* -------------------------------------------------------------------------- */

void loop() {
    ProcessReceiveQueue();
    ExpireIncompleteMessages();

    static uint32_t lastBroadcast = 0;

    if (
        static_cast<uint32_t>(
            millis() - lastBroadcast
        ) >= BROADCAST_INTERVAL_MS
    ) {
        lastBroadcast = millis();

        DeviceState state =
            CreateLocalState();

        /*
         * Show the local object before broadcasting it. This uses the same
         * local-vs-remote presentation code as received objects.
         */
        PrintDeviceState(
            state,
            localMac.data(),
            true
        );

        if (!BroadcastState(state)) {
            Serial.println(
                "ESP-NOW broadcast failed."
            );
        }
    }

    delay(1);
}
