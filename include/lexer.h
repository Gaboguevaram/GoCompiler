#ifndef LEXER_H
#define LEXER_H

#include "tokens.h"
#include "symbol_table.h"

/*
 * Lexical analyzer (scanner). Driven by next_token(), which returns one Go
 * token at a time. Internally it is a hand-written DFA: a dispatch on the
 * first character delegates to a per-category state machine.
 */

#define KEYWORDS_COUNT 25 /* number of Go reserved words */

/* A reserved word and the token code it maps to. */
typedef struct {
    char *word; /* the keyword text */
    int type;   /* its token code */
} Keyword;

extern Keyword keywords[];

/**
 * Initialise the lexer (and the underlying input buffer) for the given file.
 * @param path path to the source file to scan.
 */
void lexer_init(const char *path);

/**
 * Shut down the lexer and release the input buffer.
 */
void lexer_close(void);

/**
 * Scan and return the next token. A token with an empty lexeme ("") means a
 * comment was skipped; a token whose type is EOF marks end of input.
 * @return the next token.
 */
Token next_token(void);

#endif /* LEXER_H */
