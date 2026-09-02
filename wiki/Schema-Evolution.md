# Schema Evolution

Persisted or remotely stored data can outlive the firmware that wrote it. Serializable therefore treats schema evolution as a first-class concern.

The model declares a schema version and can use aliases, defaults, and migration helpers to restore older representations into the current schema.

```text
old representation
      |
      v
migration / aliases / defaults
      |
      v
current schema
      |
      v
validation
      |
      v
object
```

## Property renames

Use aliases/migration rather than simply changing a serialized property name and making all existing persisted data unreadable.

## Defaults

Defaults can populate newly introduced optional properties when older representations do not contain them.

## Migrations

Use migrations for structural or semantic changes that cannot be expressed as a simple alias/default.

## Testing

Keep compatibility vectors representing prior supported schemas and deserialize them with the current implementation. See [Schema and Migration Contracts](Schema-and-Migration-Contracts).