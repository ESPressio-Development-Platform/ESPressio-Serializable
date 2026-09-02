# Schema and Migration Contracts

Schema evolution must keep persisted data readable across intended firmware upgrades without making representation code responsible for model history.

## Versioning

Increment the model schema version when a representation change requires migration semantics.

## Aliases

Use aliases for compatible property renames so old serialized names can resolve to the current property.

## Defaults

Use defaults for newly introduced properties when old representations legitimately lack them.

## Migrations

Use explicit migrations for structural/semantic changes that cannot be expressed by aliases/defaults alone.

## Ordering

Migrations must produce a representation/state compatible with the current schema before final validation.

## Compatibility testing

Keep fixtures/vectors for prior supported schema versions. Test them against the current implementation through the persistence-oriented archive path, not only through hand-constructed current-schema values.

## Direct Binary

Do not rely on same-schema Direct Binary when migration is required. Use the normal structured ESPB/archive path.