# Fuzzing

Build with Clang/libFuzzer, for example:

```sh
clang++ -std=c++17 -fsanitize=fuzzer,address,undefined -Isrc tests/fuzz/fuzz_binary.cpp -o fuzz_binary
clang++ -std=c++17 -fsanitize=fuzzer,address,undefined -Isrc tests/fuzz/fuzz_cbor.cpp -o fuzz_cbor
./fuzz_binary -max_len=4096 corpus/binary
./fuzz_cbor -max_len=4096 corpus/cbor
```

JSON streaming fuzzing belongs in the ESP32/ArduinoJson test matrix because the adapter is intentionally optional and Arduino-specific. `tests/platformio/` provides malformed JSON/stream tests for that environment.
