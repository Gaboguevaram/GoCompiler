#include "errores.h"
#include <stdio.h>
#include <stdlib.h>

//Implementacion de la gestion de errores

/**
 * Cuando el tamaño maximo de lexema permitido se supera
 */
void tamanho_de_lexema_max_superado() {
    printf("ERROR: TAMANHO DE LEXEMA MAXIMO SUPERADO. NO SE HA NECESITADO TRUNCAR EL LEXEMA\n");
}

/**
 * Cuando es necesario truncar el lexema
 */
void truncado_necesario() {
    printf("ERROR: TRUNCADO DEL LEXEMA NECESARIO\n");
}

/**
 * Cuando se produce un error al abrir el archivo a analizar
 */
void error_abrir_archivo() {
    printf("ERROR AL ABRIR EL ARCHIVO\n");
    exit(1);
}

/**
 * Cuando se detecta un error lexico
 */
void error_lexico() {
    printf("ERROR LEXICO\n");
}
