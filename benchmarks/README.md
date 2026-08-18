# Benchmark matrix

The benchmark firmware lives in `examples/SerializationBenchmark`. The `platformio.ini` here defines representative build environments for classic ESP32, ESP32-S3 internal-RAM-only, and ESP32-S3 with PSRAM enabled.

For each physical target, record the exact board, Arduino-ESP32 core/platform version, CPU frequency, flash/PSRAM configuration, compiler optimization, firmware text/data sizes, and the CSV emitted by the example. Store measured results in a new subdirectory named with the target and date; do not replace measurements from different hardware with averaged or inferred figures.

The repository deliberately does not ship fabricated benchmark numbers. Host tests measure correctness; physical ESP32 runs measure embedded RAM/flash/runtime behavior.
