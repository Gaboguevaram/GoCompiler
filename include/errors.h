#ifndef ERRORS_H
#define ERRORS_H

/* Error reporting for the lexical analyzer. */

/**
 * Reported when a lexeme exceeds the maximum length but could still be read.
 */
void lexeme_too_long_error(void);

/**
 * Reported when a lexeme must be truncated to fit the buffer.
 */
void truncation_needed_error(void);

/**
 * Reported (and fatal) when the input file cannot be opened.
 */
void file_open_error(void);

/**
 * Reported on an invalid character or malformed token.
 */
void lexical_error(void);

/**
 * Reported when a string literal reaches end-of-file without a closing quote.
 */
void unterminated_string_error(void);

#endif /* ERRORS_H */
