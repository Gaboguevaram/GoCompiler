#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "symbol_table.h"
#include "lexer.h" /* keywords[] and KEYWORDS_COUNT */

/* IMPLEMENTATION OF THE SYMBOL TABLE */

static SymbolTable table; /* the symbol table instance */

/**
 * Heap-duplicates a NUL-terminated string (portable strdup).
 * @param s string to duplicate.
 * @return a newly allocated copy of [s], owned by the caller.
 */
static char *dup_string(const char *s) {
    size_t n = strlen(s) + 1;
    char *copy = malloc(n);
    memcpy(copy, s, n);
    return copy;
}

/**
 * Fills the reserved words into the symbol table.
 *
 * Each keyword string is duplicated onto the heap so the table uniformly owns
 * every lexeme it stores (see symtable_free).
 */
static void load_keywords(void) {
    for (unsigned int i = 0; i < KEYWORDS_COUNT; i++) {
        Token e;
        e.lexeme = dup_string(keywords[i].word);
        e.type = keywords[i].type;
        hashtable_insert(&table, e);
    }
}

/**
 * Initialises the symbol table and preloads the keywords.
 */
void symtable_init(void) {
    hashtable_init(table); /* initialise the hash table */
    load_keywords();       /* insert the keywords */
}

/**
 * Prints the symbol table, bucket by bucket.
 */
void symtable_print(void) {
    printf("SYMBOL TABLE:\n");
    for (int i = 0; i < TABLE_SIZE; i++) { /* walk the table printing elements */
        printf("Bucket %d: ", i);
        ListPos p = list_first(table[i]);
        while (p != list_end(table[i])) {
            Token e;
            list_get(table[i], p, &e);
            printf("[%s -> %d] -> ", e.lexeme, e.type);
            p = list_next(table[i], p);
        }
        printf("NULL\n");
    }
}

/**
 * Inserts a new identifier into the symbol table.
 * @param lexeme the heap lexeme to insert; the table takes ownership of it.
 */
static void insert_identifier(char *lexeme) {
    Token e;
    e.lexeme = lexeme;
    e.type = TOKEN_ID;
    hashtable_insert(&table, e);
}

/**
 * Looks up the token type associated with a given lexeme in the symbol table.
 *
 * Takes ownership of the heap string *lexeme: if the lexeme is already known,
 * the caller's copy is freed and *lexeme is rewritten to point at the
 * table-owned copy; otherwise the lexeme is inserted as a new identifier and
 * the table keeps the string.
 *
 * @param lexeme in/out pointer to the heap lexeme; may be replaced.
 * @return the token type associated with the lexeme (keyword code or TOKEN_ID).
 */
int symtable_lookup(char **lexeme) {
    Token e;
    if (hashtable_get(table, *lexeme, &e)) { /* already in the table */
        free(*lexeme);          /* drop the caller's duplicate */
        *lexeme = e.lexeme;     /* reuse the table-owned copy */
        return e.type;
    }
    insert_identifier(*lexeme); /* not there: insert it, table now owns it */
    return TOKEN_ID;
}

/**
 * Deletes the symbol table, freeing every lexeme it owns.
 */
void symtable_free(void) {
    hashtable_free(table); /* destroy the hash table */
}
