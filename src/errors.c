#include <stdio.h>
#include <stdlib.h>
#include "errors.h"

/* IMPLEMENTATION OF ERROR REPORTING */

/*
 * Lexical diagnostics are written to stdout so they appear in deterministic
 * order, interleaved with the token stream (which makes the test golden files
 * reproducible). The fatal file-open error goes to stderr.
 */

/**
 * Reported when the maximum allowed lexeme length is exceeded.
 */
void lexeme_too_long_error(void) {
    printf("ERROR: maximum lexeme length exceeded (lexeme not truncated)\n");
}

/**
 * Reported when the lexeme has to be truncated to fit the buffer.
 */
void truncation_needed_error(void) {
    printf("ERROR: lexeme truncation required\n");
}

/**
 * Reported when the file to scan cannot be opened. Terminates the program.
 */
void file_open_error(void) {
    fprintf(stderr, "ERROR: could not open input file\n");
    exit(1);
}

/**
 * Reported when a lexical error is detected (invalid or malformed token).
 */
void lexical_error(void) {
    printf("LEXICAL ERROR: invalid character or malformed token\n");
}

/**
 * Reported when a string literal reaches end of file without a closing quote.
 */
void unterminated_string_error(void) {
    printf("LEXICAL ERROR: unterminated string literal\n");
}
