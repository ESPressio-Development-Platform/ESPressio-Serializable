# Security Integration Contract

Serializable Security integration protects a completed representation through `Security::IDataProtector` rather than embedding cryptography into model schemas or archive field processing.

## Dependency rule

The core Serializable umbrella must remain Security-independent. Security integration belongs behind explicit integration headers and APIs.

## Processing order

Serialize normally first, then protect the finished bytes. On restore, authenticate/decrypt first, then parse/migrate/default/validate the normal archive.

## Purpose binding

Require an authenticated purpose/context appropriate to the owning domain so protected blobs cannot be substituted between unrelated records under the same key policy.

## Result separation

Preserve distinct Security and deserialization results. Authentication/key failure must not be collapsed into a normal archive parse error.

## Limits

Protection configuration must retain archive-size and decode limits. Successful authentication does not justify unbounded allocation.

## Extension rule

If another protection mechanism is added in future, adapt it at the completed-representation boundary and preserve the same separation of schema, representation, protection, and resource policy.