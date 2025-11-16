1. Put to CockroachDB source folder. 

```bash
cp ./cockroachdb_parser_helper <cockroachdb_src>/pkg/cmd/parser-helper
```

2. Compile:
```bash
bazel build pkg/cmd/parser-helper --sandbox_debug --verbose_failures
```

3. Copy the compiled binary out:
```bash
cp _bazel/bin/pkg/cmd/parser-helper/parser-helper_/parser-helper ./parser-helper
```