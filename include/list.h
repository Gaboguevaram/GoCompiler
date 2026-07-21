#ifndef LIST_H
#define LIST_H

#include "tokens.h"

/*
 * Singly linked list with a header node, used by the hash table for
 * separate chaining. Elements are Token values (see tokens.h).
 */

/* A position in the list (points to the node *before* the element). */
typedef struct cell *ListPos;
/* Handle to a list. */
typedef struct list *List;

/**
 * Allocate an empty list.
 * @param l pointer to the list handle to create.
 */
void list_create(List *l);

/**
 * Destroy the list and free every node it holds.
 * @param l pointer to the list to destroy.
 */
void list_destroy(List *l);

/**
 * Check whether the list is empty.
 * @return 1 if empty, 0 otherwise.
 */
unsigned list_is_empty(List l);

/**
 * Check whether the list handle exists (is non-NULL).
 * @return 1 if it exists, 0 otherwise.
 */
unsigned list_exists(List l);

/**
 * @return the first position of the list.
 */
ListPos list_first(List l);

/**
 * @return the end-of-list position (past the last element).
 */
ListPos list_end(List l);

/**
 * @return the position following p.
 */
ListPos list_next(List l, ListPos p);

/**
 * @return the position preceding p.
 */
ListPos list_prev(List l, ListPos p);

/**
 * Retrieve the element stored at position p.
 * @param e output pointer that receives the element.
 */
void list_get(List l, ListPos p, Token *e);

/**
 * @return the number of elements in the list.
 */
unsigned list_length(List l);

/**
 * Insert element e right after position p.
 */
void list_insert(List *l, ListPos p, Token e);

/**
 * Remove the element stored at position p.
 */
void list_remove(List *l, ListPos p);

/**
 * Overwrite the element stored at position p with e.
 */
void list_set(List *l, ListPos p, Token e);

#endif /* LIST_H */
