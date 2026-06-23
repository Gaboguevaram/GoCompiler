#include <stdio.h>
#include <stdlib.h>

#include "TS.h"
#include "Al.h"



int main(void) {

    ////////////////////////
    /////INICIALIZACIÓN/////
    ////////////////////////

    inicializar_tabla(); //Inicializamos la tabla de simbolos

    inicializar_AL(); //Inicializamos el analizador lexico, que inicializa a su vez al sistema de entrada

    imprimir_tabla(); //Imprimimos la tabla de simbolos

    printf("////////////////////////////////\n");

    ////////////////////////
    ///////DESARROLLO///////
    ////////////////////////

    TIPOELEMENTO componente;

    while (1) { //Bucle que consume los componente lexicos

        componente = sig_componente(); //Pedimos el siguiente componente

        if (strcmp(componente.nombre,"")==0) { //Si nos llega vacio (comentarios) pedimos el siguiente
            continue;
        }
        if (componente.token != EOF) { //Si no es el fin de fichero imprimimos el componente junto con el lexema
            if (strcmp(componente.nombre,"caracter")==0) {
                printf("<%d,\"%c\">\n", componente.token, componente.token);
            }else {
                printf("<%d,\"%s\">\n", componente.token, componente.nombre); //Se libera la memoria de los numeros y cadenas. Los identificadores estan en la tabla de simbolos y no se pueden borrar
                if (componente.token == 390 || (componente.token <= 379 && componente.token >= 376)) {
                    free(componente.nombre);
                    componente.nombre = NULL;
                }

            }
        }else { //Si es el EOF terminamos el analisis lexico
           break;
        }
    }

    printf("////////////////////////////////\n");

    imprimir_tabla(); //Imprimimos la tabla de simbolos nuevamente

    ////////////////////////
    //////FINALIZACION//////
    ////////////////////////

    printf("////////////////////////////////\n");
    printf("FIN DE LA EJECUCION\n");

    finalizar_AL();

    borrar_tabla_simbolos(); //Destruimos la tabla de simbolos

    exit(0); //Finalizamos la ejecucion
}
