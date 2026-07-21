# Go Lexical Analyzer

A lexical analyzer (scanner / tokenizer) for a subset of the
[Go](https://go.dev) programming language, written in portable C11.

It implements the **tokenization phase** of a compiler: it reads Go source code
and turns it into a stream of tokens (keywords, identifiers, literals,
operators, delimiters), resolving identifiers through a symbol table and
reporting lexical errors. It is **not** a parser or a full compiler — there is
no grammar, AST or code generation. The architecture is, however, deliberately
shaped so a parser can be added on top later (see [Future work](#future-work)).

The focus of the project is on the systems-level concerns of a scanner:
a hand-written finite-state machine, an efficient double-buffered reader, and
careful manual memory management with a single, well-defined ownership model.

---

## Contents

- [What it recognizes](#what-it-recognizes)
- [How it works](#how-it-works)
  - [Scanning: a hand-written DFA](#scanning-a-hand-written-dfa)
  - [Input buffering: twin buffer with sentinels](#input-buffering-twin-buffer-with-sentinels)
  - [Memory management](#memory-management)
- [Building and running](#building-and-running)
- [Example: input to token stream](#example-input-to-token-stream)
- [Lexical error handling](#lexical-error-handling)
- [Project layout](#project-layout)
- [Testing](#testing)
- [Checking for memory leaks](#checking-for-memory-leaks)
- [Design decisions / what I learned](#design-decisions--what-i-learned)
- [Future work](#future-work)
- [License](#license)

## What it recognizes

| Category        | Examples                                                                 |
|-----------------|--------------------------------------------------------------------------|
| **Keywords**    | the 25 Go reserved words: `func`, `var`, `for`, `range`, `chan`, `go`, `select`, `defer`, `map`, `struct`, `interface`, … |
| **Identifiers** | `[A-Za-z_][A-Za-z0-9_]*`, looked up / inserted in the symbol table        |
| **Integers**    | decimal `42`, hexadecimal `0xBadFace`                                     |
| **Floats**      | `3.14`, `1e-11`, `.1273E2`, `4.e+0`                                       |
| **Imaginary**   | `0i`, `1.e0i`                                                             |
| **Strings**     | `"hello, \"world\""` (with escaped quotes)                                |
| **Operators**   | `+ += ++  - -= --  * *=  / /=  % %=  & &= && &^ &^=  \| \|= \|\|  ^ ^=  == !=  < <= << <<= <-  > >= >> >>=  = :=` |
| **Delimiters**  | `( ) [ ] { } , ; .` (emitted with their ASCII value)                      |
| **Comments**    | `// line` and `/* block */` (consumed, not emitted)                       |
| **Inserted `;`**| Go's [automatic semicolon insertion](https://go.dev/ref/spec#Semicolons) is emulated at the lexer level |

Each token is reported as `<type,"lexeme">`, where `type` is a numeric code
defined in [`include/tokens.h`](include/tokens.h). Single-character delimiters
use their raw ASCII value (e.g. `;` is `59`).

## How it works

### Scanning: a hand-written DFA

The scanner is a **hand-written deterministic finite automaton**, not a
table-driven or regex-generated one. [`next_token()`](src/lexer.c) reads the
first character and dispatches to a per-category routine. The numeric and string
routines are explicit state machines (`while` loop over a `state` variable);
the operator routines are short fixed look-ahead helpers. Going character by
character keeps the control flow easy to follow and makes the maximal-munch and
look-ahead decisions explicit.

One detail worth highlighting: the scanner emulates Go's **automatic semicolon
insertion**. When a token that can end a statement is followed by a newline, the
lexer emits a synthetic `;`, just as the Go compiler does before parsing.

### Input buffering: twin buffer with sentinels

Reading one byte at a time with a system call per character would be slow. The
input module ([`src/input_buffer.c`](src/input_buffer.c)) instead uses the
classic **two-block (twin) buffer with sentinels** from the Dragon Book:

```
 physical buffer: 2*N + 2 bytes
 ┌───────────────┬───┬───────────────┬───┐
 │   block A (N) │ S │   block B (N) │ S │     S = sentinel byte (0xFF)
 └───────────────┴───┴───────────────┴───┘
   ^start  ^forward
```

Two pointers walk the buffer: `start` marks the beginning of the current lexeme
and `forward` is the look-ahead cursor. Each block ends in a sentinel byte. When
`forward` reaches a sentinel, the **other** block is refilled with a single
`fread`, so the program performs roughly one read per `N` characters instead of
one per character. The sentinel doubles as the end-of-input marker, which lets
the hot loop test a single byte instead of separately checking the buffer
length on every character. The reader also supports pushing a character back
(`unread_char`) across the block boundary, and detects the pathological case of
a lexeme longer than a whole block.

`N` (the block size) is a compile-time constant in
[`include/input_buffer.h`](include/input_buffer.h); it is intentionally small
(16) so the buffer-refill and wrap-around paths are exercised even by short
inputs.

### Memory management

The project uses manual allocation with **one owner per allocation**:

- The **symbol table** ([`src/symbol_table.c`](src/symbol_table.c)) is a hash
  table with separate chaining (FNV-1a hash, 53 buckets). It owns every lexeme
  it stores: keyword strings are duplicated on load, and identifier lexemes are
  handed over by the lexer. When an identifier is seen again, the caller's
  duplicate is freed and the pointer is rewritten to the table-owned copy.
  Everything is released once in `symtable_free()`.
- **Number and string** lexemes are heap-allocated to their exact length and
  owned by the driver, which frees them right after printing.
- **Operators and delimiters** use static string literals and are never freed.

This single-ownership discipline is what makes the analyzer leak-free under
Valgrind (see [below](#checking-for-memory-leaks)).

## Building and running

Requirements: a C11 compiler (`gcc` or `clang`) and `make`, in a POSIX-like
environment (Linux, macOS, WSL, or MSYS2 / Git Bash on Windows).

```sh
make            # build -> build/go-lexer
make run        # scan the bundled examples/concurrent_sum.go
./build/go-lexer examples/hello.go
```

By default the driver also prints the symbol table before and after scanning.
Pass `--tokens-only` to print just the token stream:

```sh
./build/go-lexer --tokens-only examples/hello.go
```

## Example: input to token stream

Input:

```go
x := 42
```

Output (`--tokens-only`):

```
<400,"x">      # identifier  (TOKEN_ID)
<375,":=">     # short variable declaration (OP_DEFINE)
<376,"42">     # integer literal (LIT_INT)
<59,";">       # semicolon inserted automatically before the newline
```

(The `#` annotations are added here for clarity; the program prints only the
`<type,"lexeme">` lines.)

## Lexical error handling

Lexical errors are reported inline, in deterministic order with the token
stream, and scanning continues so that more than one problem can be surfaced in
a single run. Detected conditions include:

- **Unterminated string** — end of input is reached before the closing quote:
  `LEXICAL ERROR: unterminated string literal`
- **Invalid / malformed token** — e.g. a malformed number such as `0z`:
  `LEXICAL ERROR: invalid character or malformed token`
- **Lexeme longer than a buffer block** — reported as a diagnostic; the lexeme
  is still returned, or truncated if it cannot fit.

A fatal `could not open input file` error is reported on stderr.

## Project layout

```
go-lexer/
├── include/          # public headers
│   ├── tokens.h          # token codes + the Token struct
│   ├── lexer.h           # scanner API (next_token)
│   ├── input_buffer.h    # twin-buffer reader API
│   ├── symbol_table.h    # symbol table API
│   ├── hash_table.h      # hash table (separate chaining)
│   ├── list.h            # linked list used by the hash table
│   └── errors.h          # error reporting
├── src/              # implementation (one .c per header + main.c)
├── examples/         # sample Go programs
├── tests/            # golden tests (cases + expected token streams)
├── Makefile
└── LICENSE
```

Each header declares one module's public interface and the matching `.c` file
implements it, so the dependency direction is easy to follow: `lexer` pulls
characters from `input_buffer` and resolves identifiers through
`symbol_table`, which is built on `hash_table` and `list`.

## Testing

Golden tests live in [`tests/`](tests/): each `cases/<name>.go` is scanned and
its token stream compared against `cases/<name>.expected`.

```sh
make test          # verify against the committed expected token streams
make test-record   # regenerate the expected files from the current build
```

The cases cover identifiers and keyword lookup, automatic semicolon insertion,
compound operators, and the two lexical-error paths above. See
[`tests/README.md`](tests/README.md) for details (including a note on how the
expected files were produced).

## Checking for memory leaks

The Makefile provides a Valgrind target:

```sh
make valgrind      # valgrind --leak-check=full --error-exitcode=1 on the example
```

Valgrind runs on Linux/macOS (and Windows via WSL); it is not available on
native Windows. Scanning [`examples/concurrent_sum.go`](examples/concurrent_sum.go)
under Valgrind 3.15 reports a clean run:

```
total heap usage: 314 allocs, 314 frees, 9,879 bytes allocated
All heap blocks were freed -- no leaks are possible
ERROR SUMMARY: 0 errors from 0 contexts
```

Every allocation is released exactly once, which is what the single-ownership
model above is there to guarantee. The project also builds warning-free with
`-Wall -Wextra -std=c11`.

## License

Released under the [MIT License](LICENSE).
