#include <stdio.h>
#include <string.h>

#include "TS.h"
#include "Al.h"

//Implementacion de la tabla de simbolos

TS tabla; //Definimos nuestra tabla de simbolos

/**
 * Rellena las palabras reservadas en la tabla de simbolos
 */
void __rellenar_keywords() {
    TIPOELEMENTO e;
    for (unsigned int i = 0; i < KEYWORDS_COUNT; i++) { //Recorre KEYWORDS_COUNT y uno a uno mete los elementos en la tabla
        e.nombre = keywords[i].palabra;
        e.token = keywords[i].token;
        InsertarHash(&tabla, e);
    }
}

/**
 * Inicializa la tabla de simbolos
 */
void inicializar_tabla() {
    InicializarTablaHash(tabla); //Inicializa la hash
    __rellenar_keywords(); //Mete las keywords
}

/**
 * Imprime la tabla de simbolos
 */
void imprimir_tabla() {
    printf("TABLA DE SIMBOLOS:\n");
    for (int i = 0; i < TABLE_SIZE; i++) { //Recorre la tabla imprmiendo los elementos
        printf("Índice %d: ", i);
        TPOSICION p = primeroLista(tabla[i]);
        while (p != finLista(tabla[i])) {
            TIPOELEMENTO elemento;
            recuperarElementoLista(tabla[i], p, &elemento);
            printf("[%s -> %d] -> ", elemento.nombre, elemento.token);
            p = siguienteLista(tabla[i], p);
        }
        printf("NULL\n");
    }
}

/**
 * Insertar un nuevo lexema en la tabla de simbolos
 * @param lexema el lexema a insertar
 */
void __insertar_identificador(char *lexema) {
    TIPOELEMENTO e; //Crea el TIPOELEMENTO
    e.nombre = lexema;
    e.token = ID;
    InsertarHash(&tabla,e); //Insercion en la hash
}

/**
 * Busca el componente lexico asociado a un determinado lexema en la tabla de simbolos
 * @param el lexema
 * @return el componente lexico asociado al lexema
 */
int buscar_elemento(char *lexema) {
    TIPOELEMENTO e;
    if (EsMiembroHash(tabla,lexema)) { //Comprueba si esta en la hash
        BuscarHash(tabla,lexema,&e); //Si esta lo devuelve
        return e.token;
    }
    __insertar_identificador(lexema); //Si no esta lo inserta
    return ID;
}

/**
 * Borra la tabla de simbolos
 */
void borrar_tabla_simbolos() {
    DestruirTablaHash(tabla); //Destruye la hash
}
