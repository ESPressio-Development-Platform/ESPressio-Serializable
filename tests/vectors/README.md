# Compatibility vectors

These files are immutable release fixtures. `v0.6.1/vector_state.binary.hex` freezes the exact native Binary v2 bytes. CBOR is also frozen here to catch accidental encoder drift; JSON is a semantic fixture (whitespace/property ordering are not compatibility requirements).

When a new release intentionally changes a native Binary layout, create a new version directory and increment the embedded Binary wire-format byte. Never rewrite an older released vector.
