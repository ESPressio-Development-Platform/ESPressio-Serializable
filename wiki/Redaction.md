# Redaction

Redaction metadata and authenticated encryption solve different problems.

**Redaction** prevents sensitive property values from being emitted in diagnostic/operator-facing representations where redaction is requested.

**Protection** encrypts/authenticates the complete serialized representation at rest or across an opaque protected channel.

A password/secret property should still be marked redacted even if persisted configuration is encrypted. Conversely, redaction does not make persisted bytes confidential.

## Extension rule

Archive implementations and diagnostic tooling must honour redaction semantics consistently. A new representation must not accidentally bypass redaction metadata merely because its encoder is implemented separately.

## Security rule

Never treat redaction as a substitute for appropriate storage/transport protection where confidentiality is required.