#include <stdlib.h>
#include "list.h"

/* SINGLY LINKED LIST WITH A HEADER NODE */

/* A node of the singly linked list. */
struct cell {
    Token element;     /* stored element */
    struct cell *next; /* next node */
};

/* List header: keeps the head node, the tail and the element count. */
struct list {
    ListPos head;      /* header node (its "next" points to the first element) */
    unsigned length;   /* number of elements */
    ListPos tail;      /* last node */
};

/**
 * Reserves memory for a list of elements of type [Token].
 * @param l pointer to the list to create.
 */
void list_create(List *l) {
    (*l) = (List) malloc(sizeof(struct list));
    (*l)->head = (ListPos) malloc(sizeof(struct cell)); /* header node */
    (*l)->tail = (*l)->head; /* tail starts at the header */
    ((*l)->tail)->next = NULL;
    (*l)->length = 0;
}

/**
 * Destroys (frees the reserved memory of) the list [l] and every node it holds.
 * @param l pointer to the list to destroy.
 */
void list_destroy(List *l) {
    (*l)->tail = (*l)->head;
    while ((*l)->tail != NULL) { /* walk the chain freeing each node */
        (*l)->tail = ((*l)->tail)->next;
        free((*l)->head);
        (*l)->head = (*l)->tail;
    }
    free(*l);
    *l = NULL; /* avoid dangling access */
}

/**
 * Checks whether the list [l] has been created.
 * @param l list to check for existence.
 * @return 1 if the list exists, 0 otherwise.
 */
unsigned list_exists(List l) {
    if (l != NULL) return 1;
    return 0;
}

/**
 * Checks whether the list [l] is empty.
 * @param l list to check for emptiness.
 * @return 1 if the list is empty, 0 otherwise.
 */
unsigned list_is_empty(List l) {
    if (l->length == 0) return 1;
    return 0;
}

/**
 * Retrieves the first position of the list.
 * @param l list from which to retrieve the first position.
 * @return the first position, of type [ListPos], of the list [l].
 */
ListPos list_first(List l) {
    return (l->head);
}

/**
 * Returns the position following [p] in the list [l].
 * @param l list in which the next position is looked up.
 * @param p reference position whose successor is returned.
 * @return the position following [p].
 */
ListPos list_next(List l, ListPos p) {
    (void) l;
    return (p->next);
}

/**
 * Retrieves the end position of the list.
 * @param l list from which to retrieve its end.
 * @return the end position, of type [ListPos], of the list [l].
 */
ListPos list_end(List l) {
    return (l->tail);
}

/**
 * Returns the position preceding [p] in the list [l].
 * @param l list in which the previous position is looked up.
 * @param p reference position whose predecessor is returned.
 * @return the position preceding [p].
 */
ListPos list_prev(List l, ListPos p) {
    ListPos q = l->head;
    while (q->next != p) { /* find the node whose successor is p */
        q = q->next;
    }
    return q;
}

/**
 * Retrieves the element stored at the position [p] passed as argument.
 * @param l list from which to retrieve the element.
 * @param p position from which to retrieve the element.
 * @param e pointer to the variable in which the retrieved element is stored.
 */
void list_get(List l, ListPos p, Token *e) {
    (void) l;
    *e = (p->next)->element;
}

/**
 * Queries the length of the list [l].
 * @param l list whose length is queried.
 * @return the number of elements in the list.
 */
unsigned list_length(List l) {
    return (l->length);
}

/**
 * Inserts the element [e] at the position following [p] in the list [l].
 * @param l pointer to the list into which the element is inserted.
 * @param p position after which the element is inserted.
 * @param e element to insert.
 */
void list_insert(List *l, ListPos p, Token e) {
    ListPos q = p->next;                        /* keep the old successor */
    p->next = (ListPos) malloc(sizeof(struct cell));
    (p->next)->element = e;
    (p->next)->next = q;

    if (q == NULL) (*l)->tail = p->next;        /* new node became the tail */
    (*l)->length++;
}

/**
 * Deletes the element at position [p] of the list [l].
 * @param l pointer to the list from which the element is deleted.
 * @param p position of the element to delete.
 */
void list_remove(List *l, ListPos p) {
    ListPos q = p->next;                 /* node holding the element to remove */
    p->next = q->next;
    if (p->next == NULL) (*l)->tail = p; /* removed the tail */
    free(q);
    (*l)->length--;
}

/**
 * Modifies the value of the element stored at position [p], storing the new
 * element [e].
 * @param l pointer to the list whose element is modified.
 * @param p position of the value to modify.
 * @param e new value to store at position [p].
 */
void list_set(List *l, ListPos p, Token e) {
    (void) l;
    (p->next)->element = e;
}
