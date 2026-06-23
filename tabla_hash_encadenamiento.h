#ifndef TABLA_HASH_ENCADENAMIENTO_H
#define TABLA_HASH_ENCADENAMIENTO_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lista.h"

//Fichero interfaz con la tabla hash

#define TABLE_SIZE 53 //Tamaño tabla

typedef TLISTA TablaHash[TABLE_SIZE]; //Definicion del tipo de dato

/**
 * Inicializa cada elemento de la tabla a una lista vacía
 * @param t tabla hash.
 */
void InicializarTablaHash (TablaHash t);

/**
 * Destruye la lista que corresponde a cada elemento de la tabla
 * @param t tabla hash.
 */
void DestruirTablaHash (TablaHash t);

/**
 * Funcion hash
 * @param cad clave para calcular la posición en la tabla hash.
 * @return la posición de cad en la tabla hash
 */
int FuncionHash(char *cad);

/**
 * Buscar una clave en la tabla hash
 * @param t es la tabla hash en la que buscamos la clave
 * @param clavebuscar: es la clave que buscamos
 * @param e es donde almacenamos el elemento encontrado
 * @return si la búsqueda tuvo éxito
 */
int BuscarHash(TablaHash t, char *clavebuscar, TIPOELEMENTO *e);

/**
 * Mira si clavebuscar está en la tabla hash
 * @param t es la tabla hash en la que buscamos la clave
 * @param clavebuscar: es la clave que buscamos
 * @return si la clave está en la tabla
 */
int EsMiembroHash (TablaHash t, char *clavebuscar);

/**
 * Inserta un elemento en una tabla hash al principio de la lista
 * @param t es la tabla hash
 * @param elemento es el elemento que queremos guardar en la tabla
 */
void InsertarHash (TablaHash *t, TIPOELEMENTO elemento);

/**
 * Borra un elemento en una tabla hash
 * @param t es la tabla hash
 * @param claveborrar es el elemento que queremos borrar en la tabla
*/
void BorrarHash (TablaHash *t, char *claveborrar);

#endif	// TABLA_HASH_ENCADENAMIENTO_H

