#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"
#include "symbol_table.h"
#include "tokens.h"

/*
 * Driver for the Go lexical analyzer.
 *
 * Usage: go-lexer [--tokens-only] <file.go>
 *
 * It prints the token stream as <type,"lexeme"> lines. By default it also
 * dumps the symbol table before and after scanning; --tokens-only suppresses
 * those dumps so the output is just the token stream (used by the tests).
 */

/**
 * Prints one token and frees its lexeme if this driver owns it.
 *
 * Number and string lexemes are heap-allocated by the lexer and handed to the
 * caller, so they are freed here. Identifier and keyword lexemes are owned by
 * the symbol table (freed by symtable_free); operators and single-character
 * delimiters use static literals.
 *
 * @param t the token to print.
 */
static void print_token(Token t) {
    if (strcmp(t.lexeme, "char") == 0) {
        printf("<%d,\"%c\">\n", t.type, t.type);
    } else {
        printf("<%d,\"%s\">\n", t.type, t.lexeme);
        if (t.type == LIT_STRING || (t.type >= LIT_INT && t.type <= LIT_IMAGINARY)) {
            free(t.lexeme);
        }
    }
}

/**
 * Entry point: parses the arguments, drives the lexer and prints the tokens.
 * @param argc number of command-line arguments.
 * @param argv command-line arguments: [--tokens-only] <file.go>.
 * @return 0 on success, 1 if no input file was given.
 */
int main(int argc, char **argv) {
    const char *path = NULL;
    int show_symtab = 1;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tokens-only") == 0) {
            show_symtab = 0;
        } else {
            path = argv[i];
        }
    }

    if (path == NULL) {
        fprintf(stderr, "usage: go-lexer [--tokens-only] <file.go>\n");
        return 1;
    }

    symtable_init(); /* initialise the symbol table (preloads keywords) */
    lexer_init(path); /* initialise the lexer and the input buffer */

    if (show_symtab) {
        symtable_print();
        printf("////////////////////////////////\n");
    }

    /* Pull tokens until end of input. */
    Token t;
    while (1) {
        t = next_token();
        if (strcmp(t.lexeme, "") == 0) {
            continue; /* a comment was skipped */
        }
        if (t.type != EOF) {
            print_token(t);
        } else {
            break;
        }
    }

    if (show_symtab) {
        printf("////////////////////////////////\n");
        symtable_print();
        printf("////////////////////////////////\n");
        printf("END OF EXECUTION\n");
    }

    lexer_close();
    symtable_free();
    return 0;
}
