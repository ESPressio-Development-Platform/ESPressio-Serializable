# Streaming

Streaming facilities allow serialization/deserialization to operate with incremental byte-oriented sources/sinks rather than requiring every representation to be materialized as one large contiguous buffer.

Use streaming for larger payloads, serial/network transports, or constrained-memory paths where whole-archive buffering is undesirable.

## Representation neutrality

Streaming is an archive/transport concern. The Serializable model declaration remains unchanged.

## Backpressure and partial I/O

A streaming adapter must define how partial reads/writes, end-of-stream, timeout/error, and retry semantics are reported. Do not hide transport failures as schema errors.

## Memory

Streaming can reduce peak contiguous-buffer pressure, but nested decode structures and collection growth still require explicit resource limits.

## Extension path

When adding a new streaming representation or sink/source adapter, preserve the archive contract and test incremental boundaries thoroughly. See [Adding Archive Types](Adding-Archive-Types).