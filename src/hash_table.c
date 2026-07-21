#include <stdlib.h>
#include <string.h>
#include "hash_table.h"

/* HASH TABLE WITH SEPARATE CHAINING */

/******* HASH FUNCTIONS *******/

/**
 * Hash function (FNV-1a).
 * @param key key used to compute the position in the hash table.
 * @return the position of key in the hash table.
 */
int hash_function(char *key) {
    unsigned int hash = 2166136261u;
    while (*key) {
        hash ^= (unsigned char) *key++;
        hash *= 16777619u;
    }
    return (int) (hash % TABLE_SIZE);
}

/**
 * Inserts an element into the hash table, at the front of its bucket list.
 * @param t the hash table.
 * @param element the element we want to store in the table.
 */
void hashtable_insert(HashTable *t, Token element) {
    int pos = hash_function(element.lexeme);
    list_insert(&(*t)[pos], list_first((*t)[pos]), element);
}

/**
 * Searches for a key in the hash table.
 * @param t the hash table in which we search for the key.
 * @param key the key we are looking for.
 * @param e where the found element is stored.
 * @return whether the search succeeded.
 */
int hashtable_get(HashTable t, char *key, Token *e) {
    ListPos p;
    int found = 0;
    Token entry;

    int pos = hash_function(key);

    p = list_first(t[pos]);
    while (p != list_end(t[pos]) && !found) {
        list_get(t[pos], p, &entry);
        if (strcmp(entry.lexeme, key) == 0) {
            found = 1;
            *e = entry;
        } else {
            p = list_next(t[pos], p);
        }
    }
    return found;
}

/**
 * Checks whether key is in the hash table.
 * @param t the hash table in which we search for the key.
 * @param key the key we are looking for.
 * @return whether the key is in the table.
 */
int hashtable_contains(HashTable t, char *key) {
    ListPos p;
    int found = 0;
    Token entry;
    int pos = hash_function(key);

    p = list_first(t[pos]);
    while (p != list_end(t[pos]) && !found) {
        list_get(t[pos], p, &entry);
        if (strcmp(key, entry.lexeme) == 0)
            found = 1;
        else {
            p = list_next(t[pos], p);
        }
    }
    return found;
}

/**
 * Deletes an element from the hash table.
 * @param t the hash table.
 * @param key the key of the element we want to delete from the table.
 */
void hashtable_remove(HashTable *t, char *key) {
    ListPos p;
    Token entry;
    int pos = hash_function(key);

    p = list_first((*t)[pos]);
    list_get((*t)[pos], p, &entry);
    while (p != list_end((*t)[pos]) && strcmp(key, entry.lexeme)) {
        p = list_next((*t)[pos], p);
        list_get((*t)[pos], p, &entry);
    }
    if (p != list_end((*t)[pos]))
        list_remove(&(*t)[pos], p);
}

/**
 * Initialises every element of the table to an empty list.
 * @param t the hash table.
 */
void hashtable_init(HashTable t) {
    for (int i = 0; i < TABLE_SIZE; i++)
        list_create(&t[i]);
}

/**
 * Frees every stored lexeme and destroys the list of each element of the table.
 *
 * The table owns the lexeme strings of its entries, so they are released here
 * exactly once. (Single ownership point -> no leaks under valgrind.)
 * @param t the hash table.
 */
void hashtable_free(HashTable t) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        ListPos p = list_first(t[i]);
        while (p != list_end(t[i])) {
            Token entry;
            list_get(t[i], p, &entry);
            free(entry.lexeme);
            p = list_next(t[i], p);
        }
        list_destroy(&t[i]);
    }
}
