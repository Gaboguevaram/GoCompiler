# Go Lexical Analyzer - build, test and memory-check targets.
#
# Requires a POSIX-like environment (Linux, macOS, WSL or MSYS2/Git Bash):
# the recipes use gcc, sh/bash, mkdir -p and rm -rf.

CC      := gcc
CFLAGS  := -std=c11 -Wall -Wextra -Iinclude -g

SRCDIR  := src
OBJDIR  := build
SRC     := $(wildcard $(SRCDIR)/*.c)
OBJ     := $(SRC:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
BIN     := $(OBJDIR)/go-lexer
EXAMPLE := examples/concurrent_sum.go

.PHONY: all build test test-record run valgrind clean

# `all` and `build` both compile the analyzer.
all build: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Rebuild every object if any public header changes (coarse but correct).
$(OBJ): $(wildcard include/*.h)

# Run the golden tests against the committed expected token streams.
test: all
	@bash tests/run_tests.sh

# Regenerate the expected token streams from the current build.
test-record: all
	@bash tests/run_tests.sh --record

# Scan the bundled example file.
run: all
	@./$(BIN) $(EXAMPLE)

# Check for memory leaks (Linux/macOS; not available on native Windows).
valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all --error-exitcode=1 ./$(BIN) $(EXAMPLE)

clean:
	rm -rf $(OBJDIR)
