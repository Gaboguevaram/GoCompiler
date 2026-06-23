#include <stdlib.h>
#include "lista.h"

// Definición de una celda de la lista enlazada
struct celda {
    TIPOELEMENTO elemento; // Dato almacenado en la celda
    struct celda *sig; // Puntero a la siguiente celda
};

// Definición de la estructura de la lista
struct l {
    TPOSICION inicio; // Puntero al primer elemento de la lista
    unsigned longitud; // Cantidad de elementos en la lista
    TPOSICION fin; // Puntero al último elemento de la lista
};

/**
 * Reserva memoria para una lista de datos con el tipo [TIPOELEMENTO].
 * @param q puntero a la lista a crear.
 */
void crearLista(TLISTA *l) {
    (*l) = (TLISTA) malloc(sizeof(struct l)); // Reserva memoria para la lista
    (*l)->inicio = (TPOSICION) malloc(sizeof(struct celda)); // Crea la celda inicial
    (*l)->fin = (*l)->inicio; // Inicialmente, fin apunta al inicio
    ((*l)->fin)->sig = NULL; // No hay elementos, así que el siguiente es NULL
    (*l)->longitud = 0; // La lista comienza vacía
}

/**
 * Destruye (libera la memoria reservada) la lista [l] y todos los elementos que almacena.
 * @param l puntero a la lista a destruir.
 */
void destruirLista(TLISTA *l) {
    (*l)->fin = (*l)->inicio;
    while ((*l)->fin != NULL) { // Recorre la lista liberando memoria
        (*l)->fin = ((*l)->fin)->sig;
        free((*l)->inicio);
        (*l)->inicio = (*l)->fin;
    }
    free(*l); // Libera la estructura principal de la lista
    *l = NULL; // Pone el puntero a NULL para evitar accesos indebidos
}

/**
 * Comprueba si la lista [l] está creada.
 * @param l lista a comprobar si existe.
 * @return 1 si la lista existe, 0 en otro caso.
 */
unsigned existeLista(TLISTA l) {
    if (l != NULL) return 1;
    return 0;
}

/**
 * Comprueba si la lista [l] esta vacia.
 * @param l lista a comprobar si esta vacia.
 * @return 1 si la lista esta vacia, 0 en otro caso.
 */
unsigned esListaVacia(TLISTA l) {
    if (l->longitud == 0) return 1;
    return 0;
}

/**
 * Recupera la primera posicion de la lista.
 * @param l lista de la cual recuperar la primera posicion.
 * @return la primera posicion tipo [TPOSICION] de la lista [l].
 */
TPOSICION primeroLista(TLISTA l) {
    return (l->inicio);
}

/**
 * Devuelve la posicion siguiente a [p] en la lista [l].
 * @param l lista en la cual se va a buscar la siguiente posicion.
 * @param p posicion referencia para devolver la siguiente.
 * @return la posicion siguiente a [p].
 */
TPOSICION siguienteLista(TLISTA l, TPOSICION p) {
    return (p->sig);
}

/**
 * Recupera la posicion del fin de la lista.
 * @param l lista de la cual recuperar su final.
 * @return la posicion del fin tipo [TPOSICION] de la lista [l].
 */
TPOSICION finLista(TLISTA l) {
    return (l->fin);
}

/**
 * Devuelve la posicion anterior a [p] en la lista [l].
 * @param l lista en la cual se va a buscar la anterior posicion.
 * @param p posicion referencia para devolver la anterior.
 * @return la posicion siguiente a [p].
 */
TPOSICION anteriorLista(TLISTA l, TPOSICION p) {
    TPOSICION q = l->inicio;
    while (q->sig != p) { // Busca el nodo anterior al dado
        q = q->sig;
    }
    return q;
}

/**
 * Recupera el elemento almacenado en la posicion [p] pasada por argumento.
 * @param l lista de la cual recuperar el elemento.
 * @param p posicion de la cual recuperar el elemento.
 * @param e puntero a la variable en la cual almacenar el elemento recuperado.
 */
void recuperarElementoLista(TLISTA l, TPOSICION p, TIPOELEMENTO *e) {
    *e = (p->sig)->elemento;
}

/**
 * Consulta la longitud de la lista [l].
 * @param l lista de la cual consultar la longitud.
 * @return entero con el valor de la longitud de la lista.
 */
unsigned longitudLista(TLISTA l) {
    return (l->longitud);
}

/**
 * Inserta el elemento [e] en la posicion siguiente a la posicion [p] de la lista [l].
 * @param l puntero a la lista en la cual se va a insertar el elemento.
 * @param p posicion despues de la cual se insertara el elemento.
 * @param e elemento a insertar.
 */
void insertarElementoLista(TLISTA *l, TPOSICION p, TIPOELEMENTO e) {
    TPOSICION q = p->sig; // Guarda la referencia al siguiente nodo
    p->sig = (TPOSICION) malloc(sizeof(struct celda)); // Crea un nuevo nodo
    (p->sig)->elemento = e; // Almacena el nuevo elemento
    (p->sig)->sig = q; // Conecta el nuevo nodo con el siguiente

    if (q == NULL) (*l)->fin = p->sig; // Si el nuevo nodo es el último, actualiza fin
    (*l)->longitud++; // Incrementa la longitud de la lista
}

/**
 * Suprime el elemento en posicion [p] de la lista [l].
 * @param l puntero a la lista de la que se suprimira el elemento.
 * @param p posicion del elemento a suprimir.
 */
void suprimirElementoLista(TLISTA *l, TPOSICION p) {
    TPOSICION q = p->sig; // Guarda la referencia al nodo a eliminar
    p->sig = q->sig; // Conecta el nodo anterior con el siguiente
    if (p->sig == NULL) (*l)->fin = p; // Si eliminamos el último, actualizamos fin
    free(q); // Liberamos la memoria del nodo eliminado
    (*l)->longitud--; // Decrementamos la longitud de la lista
}

/**
 * Modifica el valor del elemento almacenado en la posicion [p] guardando el nuevo elemento [e].
 * @param l puntero a la lista de la cual se va a modificar el elemento.
 * @param p posicion del valor que se va a modificar.
 * @param e nuevo valor a guardar en la posicion [p].
 */
void modificarElementoLista(TLISTA *l, TPOSICION p, TIPOELEMENTO e) {
    (p->sig)->elemento = e; // Actualiza el elemento en la posición dada
}
