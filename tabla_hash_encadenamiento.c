#include "tabla_hash_encadenamiento.h"

/* TABLA HASH CON ENCADENAMIENTO */

/******* FUNCIONES HASH *******/

/**
 * Funcion hash
 * @param cad clave para calcular la posición en la tabla hash.
 * @return la posición de cad en la tabla hash
 */
int FuncionHash(char *cad) {
    unsigned int hash = 2166136261u;
    while (*cad) {
        hash ^= (unsigned char)*cad++;
        hash *= 16777619;
    }
    return hash % TABLE_SIZE;
}

/**
 * Inserta un elemento en una tabla hash al principio de la lista
 * @param t es la tabla hash
 * @param elemento es el elemento que queremos guardar en la tabla
 */
void InsertarHash(TablaHash *t, TIPOELEMENTO elemento) {
    int pos = FuncionHash(elemento.nombre);
    insertarElementoLista(&(*t)[pos], primeroLista((*t)[pos]), elemento);
}

/**
 * Buscar una clave en la tabla hash
 * @param t es la tabla hash en la que buscamos la clave
 * @param clavebuscar: es la clave que buscamos
 * @param e es donde almacenamos el elemento encontrado
 * @return si la búsqueda tuvo éxito
 */
int BuscarHash(TablaHash t, char *clavebuscar, TIPOELEMENTO *e) {
    TPOSICION p;
    unsigned int encontrado = 0;
    TIPOELEMENTO ele;

    int pos = FuncionHash(clavebuscar);

    p = primeroLista(t[pos]);
    while (p != finLista(t[pos]) && !encontrado) {
        recuperarElementoLista(t[pos], p, &ele);
        if (strcmp(ele.nombre, clavebuscar) == 0) {
            encontrado = 1;
            *e = ele;
        } else {
            p = siguienteLista(t[pos], p);
        }
    }
    return encontrado;
}

/**
 * Mira si clavebuscar está en la tabla hash
 * @param t es la tabla hash en la que buscamos la clave
 * @param clavebuscar: es la clave que buscamos
 * @return si la clave está en la tabla
 */
int EsMiembroHash(TablaHash t, char *clavebuscar) {
    TPOSICION p;
    int encontrado = 0;
    TIPOELEMENTO elemento;
    int pos = FuncionHash(clavebuscar);
    p = primeroLista(t[pos]);
    while (p != finLista(t[pos]) && !encontrado) {
        recuperarElementoLista(t[pos], p, &elemento);
        if (strcmp(clavebuscar, elemento.nombre) == 0)
            encontrado = 1;
        else {
            p = siguienteLista(t[pos], p);
        }
    }
    return encontrado;
}

/**
 * Borra un elemento en una tabla hash
 * @param t es la tabla hash
 * @param claveborrar es el elemento que queremos borrar en la tabla
*/
void BorrarHash(TablaHash *t, char *claveborrar) {
    TPOSICION p;
    TIPOELEMENTO elemento;
    int pos = FuncionHash(claveborrar);

    p = primeroLista((*t)[pos]);
    recuperarElementoLista((*t)[pos], p, &elemento);
    while (p != finLista((*t)[pos]) && strcmp(claveborrar, elemento.nombre)) {
        p = siguienteLista((*t)[pos], p);
        recuperarElementoLista((*t)[pos], p, &elemento);
    }
    if (p != finLista((*t)[pos]))
        suprimirElementoLista(&(*t)[pos], p);
}

/**
 * Inicializa cada elemento de la tabla a una lista vacía
 * @param t tabla hash.
 */
void InicializarTablaHash(TablaHash t) {
    for (int i = 0; i < TABLE_SIZE; i++)
        crearLista(&t[i]);
}

/**
 * Destruye la lista que corresponde a cada elemento de la tabla
 * @param t tabla hash.
 */
void DestruirTablaHash(TablaHash t) {
    for (int i = 0; i < TABLE_SIZE; i++)
        destruirLista(&t[i]);
}
