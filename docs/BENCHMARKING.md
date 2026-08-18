# ESP32 benchmarking

`examples/SerializationBenchmark` emits CSV suitable for comparison across targets. Run the same firmware/compiler optimization level on representative boards, e.g. classic ESP32 (internal RAM), ESP32-S3 without PSRAM, and ESP32-S3 with PSRAM.

Record: board/core version, CPU frequency, flash mode, optimization flags, PSRAM configuration, payload item count, encoded bytes, serialize/deserialize microseconds, free-heap delta, minimum free heap, and final firmware text/data sizes from PlatformIO's build report.

No hardware benchmark numbers are committed by the library unless they were produced on identified physical hardware; the repository contains the reproducible harness, not invented measurements.
