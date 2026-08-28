# Protected Serialization

Protected serialization applies authenticated encryption to the **complete finished serialized archive** through the optional ESPressio Security integration.

```text
object
  -> ESPB BinaryArchive
  -> Security::IDataProtector
  -> protected bytes
```

Restore performs the reverse order before schema migration/defaults/validation.

## Optional integration

Core Serializable remains Security-independent. Include the explicit Security integration header only where protected serialization is required.

## Protection context

`SerializationProtectionConfig` contains the configured protector plus an authenticated purpose/context string and archive/resource limits.

Use a stable purpose such as:

```text
ESPressio.WiFi.Configuration
```

so a protected blob for one domain cannot be validly substituted into another domain using the same key.

## Protection versus schema

Encryption does not replace schema evolution. After successful authentication/decryption, the normal ESPB archive is still processed through migration, aliases, defaults and validation.

## Error separation

Keep Security failures distinct from malformed archive/schema failures so callers can respond correctly to authentication/key problems.