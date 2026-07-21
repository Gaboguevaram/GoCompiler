#include <stdio.h>
#include <stdlib.h>

#include "input_buffer.h"
#include "errors.h"

/* IMPLEMENTATION OF THE INPUT SYSTEM */

/* Input buffer state (see input_buffer.h for the high-level description). */
typedef struct {
    char *buffer;   /* physical buffer: blocks A and B back to back */
    char *start;    /* start of the current lexeme */
    char *forward;  /* lookahead cursor */
    int retreated;  /* set when unread_char crossed a block boundary */
    int truncated;  /* set when the current lexeme had to be truncated */
} InputBuffer;

static FILE *src;     /* source file being scanned */
static InputBuffer b; /* the single input-buffer instance */

/**
 * Initialises the input system: opens the file to scan and fills the first
 * logical block, placing the sentinels that close each block.
 * @param path path to the source file to scan.
 */
void input_open(const char *path) {
    src = fopen(path, "r");
    if (src == NULL) {
        file_open_error();
    }
    b.buffer = (char *) calloc((2 * BLOCK_SIZE + 2), sizeof(char));

    b.start = b.buffer;
    b.forward = b.buffer;

    b.retreated = 0;
    b.truncated = 0;

    size_t read = fread(b.buffer, sizeof(char), BLOCK_SIZE, src);
    if (read < (size_t) BLOCK_SIZE) {
        b.buffer[read] = EOF_SENTINEL; /* short file: mark the real end */
    }

    b.buffer[BLOCK_SIZE] = EOF_SENTINEL;     /* sentinel closing block A */
    b.buffer[2 * BLOCK_SIZE + 1] = EOF_SENTINEL; /* sentinel closing block B */
}

/**
 * Refill the requested logical block from the file.
 * @param block 'A' for the first block, 'B' for the second.
 */
static void fill_block(char block) {
    size_t read;
    switch (block) {
        case 'A': /* logical block A */
            if (b.start <= &b.buffer[BLOCK_SIZE] && b.truncated != 1) {
                /* `start` still points into A: the lexeme will be truncated */
                b.truncated = 1;
                truncation_needed_error();
            }
            read = fread(b.buffer, sizeof(char), BLOCK_SIZE, src);
            if (read < (size_t) BLOCK_SIZE) {
                b.buffer[read] = EOF_SENTINEL; /* real end of file */
            }
            b.forward = b.buffer; /* jump forward to the start of A */
            break;
        case 'B': /* logical block B */
            if (b.start > &b.buffer[BLOCK_SIZE] && b.truncated != 1) {
                /* `start` still points into B: the lexeme will be truncated */
                b.truncated = 1;
                truncation_needed_error();
            }
            read = fread(b.buffer + BLOCK_SIZE + 1, sizeof(char), BLOCK_SIZE, src);
            if (read < (size_t) BLOCK_SIZE) {
                b.buffer[BLOCK_SIZE + 1 + read] = EOF_SENTINEL; /* real end of file */
            }
            b.forward++; /* jump forward to the start of B */
            break;
    }
}

/**
 * Reads the next character of the block, refilling the opposite block when a
 * block-boundary sentinel is reached.
 * @return the character read (EOF_SENTINEL at end of input).
 */
char read_char(void) {
    char c = *b.forward;
    if (c == EOF_SENTINEL) {
        if (b.retreated != 1) {
            /* No retreat happened, so this must be the real end of file:
               the scanner always skips over the block sentinels otherwise. */
            return EOF_SENTINEL;
        } else {
            /* A retreat happened: figure out which block sentinel we hit. */
            if (b.forward == &b.buffer[BLOCK_SIZE]) { /* sentinel of A */
                b.forward++;
                c = *b.forward;
                b.forward++;
                return c;
            }
            if (b.forward == &b.buffer[2 * BLOCK_SIZE + 1]) { /* sentinel of B */
                b.forward = b.buffer;
                c = *b.forward;
                b.forward++;
                return c;
            }
            b.retreated = 0;
        }
    }

    b.forward++;
    if (*b.forward == EOF_SENTINEL) { /* landed on a sentinel: which block? */
        if (b.forward == &b.buffer[BLOCK_SIZE]) { /* sentinel of A */
            if (!b.retreated) {
                fill_block('B');
            } else {
                b.forward++;
                b.retreated = 0;
            }
        }
        if (b.forward == &b.buffer[2 * BLOCK_SIZE + 1]) { /* sentinel of B */
            if (!b.retreated) {
                fill_block('A');
            } else {
                b.forward = b.buffer;
                b.retreated = 0;
            }
        }
    }
    return c;
}

/**
 * Steps back one character in the block, wrapping around to the previous
 * logical block (and skipping its sentinel) when at a block boundary.
 */
void unread_char(void) {
    if (b.forward == &b.buffer[0]) {
        /* at the start of A: wrap to the end of B (skipping its sentinel) */
        b.forward = &b.buffer[2 * BLOCK_SIZE];
        b.retreated = 1;
    } else if (b.forward == &b.buffer[BLOCK_SIZE + 1]) {
        /* at the start of B: wrap to the end of A (skipping its sentinel) */
        b.forward--;
        b.forward--;
        b.retreated = 1;
    } else {
        b.forward--;
    }
}

/**
 * Returns the last lexeme read, as a freshly allocated NUL-terminated string
 * built from the characters between `start` and `forward`.
 * @return the lexeme; the caller owns the returned memory.
 */
char *get_lexeme(void) {
    char *lexeme;
    int distance = 0;

    /* TRUNCATION: rewind `start` BLOCK_SIZE characters across the circular
       buffer to recover as much of the over-long lexeme as possible.
       This has to run BEFORE the span is measured: it is the repositioned
       `start` that the copy loop below walks, so sizing the allocation from
       the old `start` would under-allocate and overflow the buffer. */
    if (b.truncated == 1) {
        b.truncated = 0;
        b.start = b.forward;
        int x = BLOCK_SIZE;
        while (x != 0) {
            b.start--;
            x--;
            if (b.start == &b.buffer[BLOCK_SIZE]) {
                b.start--; /* skip the sentinel of A */
            } else if (b.start == &b.buffer[0]) {
                b.start = &b.buffer[2 * BLOCK_SIZE]; /* wrap to end of B */
            }
        }
    }

    if (b.forward >= b.start) { /* lexeme lies in one contiguous run */
        distance = (int) (b.forward - b.start);
    } else { /* lexeme wraps around the end of the physical buffer */
        distance = (int) (((b.buffer + (2 * BLOCK_SIZE + 2)) - b.start) +
                          (b.forward - b.buffer));
    }
    lexeme = (char *) malloc((distance + 1) * sizeof(char));

    int len = 0;          /* real length, skipping sentinels and stripped chars */
    lexeme[distance] = '\0';

    /* Copy the lexeme, dropping whitespace, sentinels and escape backslashes. */
    while (b.forward != b.start) {
        if (*b.start != '\n' && *b.start != '\t' && *b.start != '\r' &&
            *b.start != EOF_SENTINEL && *b.start != '\\') {
            lexeme[len] = *b.start;
            len++;
        }
        b.start++;
        if (b.start == b.buffer + 2 * BLOCK_SIZE + 1) { /* hit sentinel of B */
            b.start = b.buffer; /* wrap to the start of A */
        }
    }

    lexeme = realloc(lexeme, (len + 1) * sizeof(char)); /* shrink to exact size */
    lexeme[len] = '\0';
    if (len > BLOCK_SIZE) {
        /* Read a lexeme longer than a block without needing truncation. */
        lexeme_too_long_error();
    }
    return lexeme;
}

/**
 * Adjusts the `start` position so it equals `forward`, discarding the
 * characters in between.
 */
void sync_lexeme_start(void) {
    b.start = b.forward;
}

/**
 * Finalises the input system: frees the buffer and closes the file.
 */
void input_close(void) {
    free(b.buffer);
    fclose(src);
}
