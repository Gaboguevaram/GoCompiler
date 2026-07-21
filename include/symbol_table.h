#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "hash_table.h"

/*
 * Symbol table. Implemented on top of the hash table, but the rest of the
 * program only ever talks to it through this interface.
 *
 * Ownership: every lexeme stored in the table lives on the heap and is
 * owned by the table. Keywords are duplicated on insertion; identifier
 * lexemes are handed over by the lexer. symtable_free releases them all.
 */

typedef HashTable SymbolTable; /* hide that a hash table is used underneath */

/**
 * Initialise the symbol table and preload the Go keywords.
 */
void symtable_init(void);

/**
 * Print the contents of the symbol table (diagnostic / demo output).
 */
void symtable_print(void);

/**
 * Resolve a lexeme to its token type.
 *
 * Takes ownership of the heap string *lexeme:
 *   - if the lexeme is already known, frees the caller's copy and rewrites
 *     *lexeme to point at the table-owned copy (so the caller keeps a valid
 *     pointer without leaking the duplicate);
 *   - otherwise inserts it as a new identifier (the table keeps the string).
 *
 * @param lexeme in/out pointer to the heap lexeme; may be replaced.
 * @return the token type (a keyword code, or TOKEN_ID).
 */
int symtable_lookup(char **lexeme);

/**
 * Destroy the symbol table and free every lexeme it owns.
 */
void symtable_free(void);

#endif /* SYMBOL_TABLE_H */
