
#ifndef TS_H
#define TS_H
#include "tabla_hash_encadenamiento.h"

//Fichero con el que se accede la tabla de simbolos

typedef TablaHash TS; //Definimos el tipo TS para encubrir que usamos una hash

/**
 * Inicializa la tabla de simbolos
 */
void inicializar_tabla();

/**
 * Imprime la tabla de simbolos
 */
void imprimir_tabla();

/**
 * Busca el componente lexico asociado a un determinado lexema en la tabla de simbolos
 * @param el lexema
 * @return el componente lexico asociado al lexema
 */
int buscar_elemento(char *lexema);

/**
 * Borra la tabla de simbolos
 */
void borrar_tabla_simbolos();

#endif //TS_H
