#ifndef SE_H
#define SE_H

//Fichero .h del sistema de entrada

#define archivo "concurrentSum.go" //Se define el archivo a analizar

#define N 16 //Tamaño de cada bloque logico


/**
 * Inicializa el sistema de entrada
 */
void inicializar_SE();

/**
 * Lee el siguiente caracter del bloque
 * @return el caracter leido
 */
char leer_caracter();

/**
 * Retrocede un caracter en el bloque
 */
void retroceder_caracter();

/**
 * Devuelve el ultimo lexema leido
 * @return el lexema leido
 */
char *devolver_lexema();

/**
 * Ajusta la posicion de inicio para que sea igual a la de delantero
 */
void ajustar_inicio();

/**
 * Finaliza el sistema de entrada
 */
void finalizar_SE();
#endif //SE_H
