#!/usr/bin/env bash
#
# Golden tests for the Go lexical analyzer.
#
# For every tests/cases/*.go file, run `go-lexer --tokens-only` and compare the
# token stream against the matching *.expected file.
#
#   tests/run_tests.sh            # verify against the committed .expected files
#   tests/run_tests.sh --record   # (re)generate the .expected files from output
#
# A missing .expected file is recorded automatically, so adding a new case is
# just dropping a .go file into tests/cases/.
#
set -u

here="$(cd "$(dirname "$0")" && pwd)"
root="$(cd "$here/.." && pwd)"
bin="$root/build/go-lexer"
cases="$here/cases"

record=0
[ "${1:-}" = "--record" ] && record=1

if [ ! -x "$bin" ]; then
    echo "error: $bin not found - run 'make' first" >&2
    exit 2
fi

pass=0
fail=0

for go in "$cases"/*.go; do
    name="$(basename "$go" .go)"
    expected="$cases/$name.expected"
    got="$("$bin" --tokens-only "$go")"

    if [ "$record" -eq 1 ] || [ ! -f "$expected" ]; then
        printf '%s\n' "$got" > "$expected"
        echo "recorded $name"
        continue
    fi

    if [ "$got" = "$(cat "$expected")" ]; then
        echo "PASS  $name"
        pass=$((pass + 1))
    else
        echo "FAIL  $name"
        diff <(cat "$expected") <(printf '%s\n' "$got") | sed 's/^/      /'
        fail=$((fail + 1))
    fi
done

echo "-----------------------------"
echo "passed: $pass  failed: $fail"
[ "$fail" -eq 0 ]
