# Schema documentation outputs

`SchemaInspector<T>` derives documentation from the same property metadata used at runtime. It currently provides `Markdown()`, `Json()`, `Csv()`, and `Mermaid()` output. These are documentation/introspection formats, not serialization wire formats, and therefore may evolve independently of Binary/CBOR compatibility provided the represented schema meaning is preserved.
