#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#include "list.h"

/*
 * Fixed-size hash table with separate chaining.
 *
 * Each bucket is a linked list of Token entries. The table *owns* the
 * lexeme string of every entry it stores: hashtable_free releases them.
 */

#define TABLE_SIZE 53 /* number of buckets (prime) */

typedef List HashTable[TABLE_SIZE];

/**
 * Initialise every bucket to an empty list.
 */
void hashtable_init(HashTable t);

/**
 * Free every stored lexeme and destroy every bucket list.
 */
void hashtable_free(HashTable t);

/**
 * FNV-1a hash of a NUL-terminated key.
 * @return the bucket index for key.
 */
int hash_function(char *key);

/**
 * Look up key and copy the matching entry into e.
 * @return 1 if found, 0 otherwise.
 */
int hashtable_get(HashTable t, char *key, Token *e);

/**
 * @return 1 if key is present in the table, 0 otherwise.
 */
int hashtable_contains(HashTable t, char *key);

/**
 * Insert element at the front of its bucket list.
 */
void hashtable_insert(HashTable *t, Token element);

/**
 * Remove the entry whose lexeme equals key (if present).
 */
void hashtable_remove(HashTable *t, char *key);

#endif /* HASH_TABLE_H */
