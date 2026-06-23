#include "definiciones.h"

#ifndef AL_H
#define AL_H

#define KEYWORDS_COUNT 25 //Numero de palabras clave

#include "TS.h"

//Fichero de acceso al analizador lexico

typedef struct { //Definicion de la estructura para guardar las palabras clave
    char *palabra; //Palabra
    int token; //Componente lexico
} Keyword;

extern Keyword keywords[];

/**
 * Inicializa el analizador lexico, inicializa el sistema de entrada
 */
void inicializar_AL();

/**
 * Finaliza el analizador lexico, finaliza el sistema de entrada
 */
void finalizar_AL();

/**
 * Retorna el siguiente componente lexico
 * @return siguiente componente lexico
 */
TIPOELEMENTO sig_componente();


#endif //AL_H
