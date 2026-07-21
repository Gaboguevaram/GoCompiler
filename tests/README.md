# Tests

Golden tests for the lexer. Each `cases/<name>.go` is scanned with
`go-lexer --tokens-only` and its output is compared to `cases/<name>.expected`.

Run them with:

```sh
make test          # build and verify against the committed .expected files
make test-record   # regenerate the .expected files from the current build
```

## Cases

| Case                     | Exercises                                              |
|--------------------------|-------------------------------------------------------|
| `assignment`             | identifier, `:=`, integer literal, inserted `;`       |
| `keyword_decl`           | keyword lookup (`var`) vs. plain identifiers          |
| `compound_op`            | compound operator (`+=`)                              |
| `unterminated_string`    | lexical error: string with no closing quote           |
| `invalid_token`          | lexical error: malformed numeric token                |

The token codes are defined in [`include/tokens.h`](../include/tokens.h);
single-character delimiters (such as `;`) are reported with their ASCII value.

## Adding a case

Drop a `.go` file into `cases/` and run `make test`: a missing `.expected` is
recorded automatically on the first run. From then on it is a regression
baseline. Use `make test-record` to re-record every case after an intentional
change to the token stream, then review the `git diff`.
