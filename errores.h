#ifndef ERRORES_H
#define ERRORES_H

//Fihcero interfaz para la gestion de errores

/**
 * Cuando el tamaño maximo de lexema permitido se supera
 */
void tamanho_de_lexema_max_superado();

/**
 * Cuando es necesario truncar el lexema
 */
void truncado_necesario();

/**
 * Cuando se produce un error al abrir el archivo a analizar
 */
void error_abrir_archivo();

/**
 * Cuando se detecta un error lexico
 */
void error_lexico();
#endif //ERRORES_H
