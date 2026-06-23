#include "SE.h"
#include "errores.h"
#include <stdio.h>
#include <stdlib.h>

#include <ctype.h>
#include <string.h>

//Implementacion sistema de entrada

FILE *arq; //Puntero al archivo a analizar

typedef struct { //Estructura con todos los elementos del sistema de entrada
    char *buffer; //Bloque fisico dividido en dos bloques logicos: A y B
    char *inicio; //Puntero inicio
    char *delantero; //Puntero delantero
    int retroceso; //Variable que indica si al retroceder se ha cambiado de bloque
    int truncado; //Variable que indica si es necesario truncar el lexema
} sistema_entrada;

sistema_entrada s; //Variable global del sistema de entrada

/**
 * Inicializa el sistema de entrada
 */
void inicializar_SE() {
    arq = fopen(archivo, "r"); //Se abre el archivo a analizar
    if (arq == NULL) {
        error_abrir_archivo();
    }
    s.buffer = (char *) calloc((2*N+2),sizeof(char)); //Se reserva memoria para los dos bloques

    s.inicio = s.buffer; //Se posiciona a inicio y a delantero al inicio del primer bloque
    s.delantero = s.buffer;

    s.retroceso = 0; //Se inicializan las variables retroceso y truncado
    s.truncado = 0;

    fread(s.buffer, sizeof(char), N, arq); //Se rellena el primer bloque

    s.buffer[N] = EOF; //Se ponen los EOFS de cada bloque
    s.buffer[2*N+1] = EOF;
}

/**
 * Rellena el bloque logico indicado
 * @param bloque bloque a rellenar
 */
void __rellenar_bloque(char bloque) {
    size_t leidos;
    switch (bloque) {
        case 'A': //Bloque logico A
            if (s.inicio <= &s.buffer[N] && s.truncado != 1) { //Si inicio esta en A, se va a tener que truncar el lexema
                s.truncado = 1; //Se pone truncado a 1
                truncado_necesario(); //Se manda error
            }
            leidos = fread(s.buffer, sizeof(char), N, arq); //Se rellena el bloque
            if (leidos < N) { //Si se ha leido menos de lo pedido hemos llegado al final
                s.buffer[leidos] = EOF; //Se pone el EOF del final del archivo tras el ultimo caracter leido
            }
            s.delantero = s.buffer; //Se avanza delantero al inicio de A
            break;
        case 'B': //Bloque logico B
            if (s.inicio > &s.buffer[N]  && s.truncado != 1) { //Si inicio esta en B, se va a tener que truncar el lexema
                s.truncado = 1; //Se pone truncado a 1
                truncado_necesario(); //Se manda error
            }
            leidos = fread(s.buffer+N+1, sizeof(char), N, arq);
            if (leidos < N) { //Si se ha leido menos de lo pedido hemos llegado al final
                s.buffer[N+1+leidos] = EOF; //Se pone el EOF del final del archivo tras el ultimo caracter leido
            }
            s.delantero++; //Se avanza delantero al inicio de B
            break;
    }

}

/**
 * Lee el siguiente caracter del bloque
 * @return el caracter leido
 */
char leer_caracter() {
    char c = *s.delantero; //Se guarda el caracter
    if (c == '\377') { //Si es un EOF
        if (s.retroceso != 1) { //Si no ha habido retroceso, obligatoriamente tiene que ser el de final del archivo,
                                // puesto que cuando se avanza siempre se salta este caracter el analizador
            return EOF; //Se devuelve EOF
        }else{ //Si ha habido retroceso, se comprueba cual EOF hemos leido
            if (s.delantero == &s.buffer[N]) { //EOF de A
                s.delantero++; //Se ajusta delantero y se guarda un caracter nuevo
                c = *s.delantero;
                s.delantero++;
                return c;
            }
            if (s.delantero == &s.buffer[2*N+1]) { //EOF de B
                s.delantero = s.buffer; //Se ajusta delantero y se guarda un caracter nuevo
                c = *s.delantero;
                s.delantero++;
                return c;
            }
            s.retroceso = 0; //En ambos casos, retroceso vuelve a valer 0, puesto
        }
    }

    s.delantero++; //Avanzamos delantero
    if (*s.delantero == '\377') { //Si pillamos un EOF, averiguamos de que bloque
        if (s.delantero == &s.buffer[N]) { //EOF de A
            if (!s.retroceso) { //Si no ha habido un retroceso, rellenamos B
                __rellenar_bloque('B');
            }else { //Sino simplemente ajustamos delantero y ponemos retroceso a 0
                s.delantero++;
                s.retroceso = 0;
            }
        }
        if (s.delantero == &s.buffer[2*N+1]) { //EOF de B
            if (!s.retroceso) { //Si no ha habido un retroceso, rellenamos A
                __rellenar_bloque('A');
            }else { //Sino simplemente ajustamos delantero y ponemos retroceso a 0
                s.delantero = s.buffer;
                s.retroceso = 0;
            }
        }
    }
    return c; //Devolvemos el caracter leido
}

/**
 * Retrocede un caracter en el bloque
 */
void retroceder_caracter() {
    if (s.delantero == &s.buffer[0]) { //Si al retroceder estabamos al principio de A, volvemos al final de B (omitiendo su EOF)
        s.delantero = &s.buffer[2*N];
        s.retroceso = 1; //Retroceso a 1 para evitar rellenado al avanzar
    }else if (s.delantero == &s.buffer[N+1]) { //Si al retroceder estabamos al principio de B, volvemos al final de A (omitiendo su EOF)
        s.delantero--;
        s.delantero--;
        s.retroceso = 1; //Retroceso a 1 para evitar rellenado al avanzar
    }else { //En cualquier otro caso, simplemente nos movemos un caracter atras
        s.delantero--;
    }
}

/**
 * Devuelve el ultimo lexema leido
 * @return el lexema leido
 */
char *devolver_lexema() {
    char *lexema;
    int distancia = 0;
    if (s.delantero >= s.inicio) {  //Calculamos el tamaño del lexema en funcion de las posiciones de inicio y delantero
        distancia = s.delantero - s.inicio;
        lexema = (char *)malloc( (distancia+1)*sizeof(char));
    }else {
        distancia = ((s.buffer+(2*N+2))-s.inicio)+(s.delantero-s.buffer);
        lexema = (char *)malloc ((distancia+1)*sizeof(char));
    }

    int len = 0; //Longitud real de nuestro lexema (obviando EOFs y caracteres invalidos)
    lexema[distancia] = '\0'; //Ponemos un \0 provisional

    //TRUNCADO

    if (s.truncado == 1) { //Si es necesario truncado
        s.truncado = 0;
        s.inicio = s.delantero; //Ponemos inicio en delantero
        int x = N;
        while (x != 0) { //Retrocedemos inicio N veces, teniendo en cuenta que el bloque completo fisico es circular
            s.inicio--;
            x--;
            if (s.inicio == &s.buffer[N]) { //Si inicio esta en el EOF de A, da un paso atras extra porque este caracter no cuenta
                s.inicio--;
            }else if (s.inicio == &s.buffer[0]) { //Si inicio esta al final de A, vuelve al final de B (omitiendo su EOF)
                s.inicio = &s.buffer[2*N];
            }
        }
    }

    //OBTENCION DEL LEXEMA

    while (s.delantero != s.inicio) { //Mientras inicio no sea igual a delantero

        //Evitamos leer caracteres invalidos o EOFs
        if (*s.inicio != '\n' && *s.inicio != '\t' && *s.inicio != '\r' && *s.inicio != '\377' && *s.inicio != '\\') {
            lexema[len] = *s.inicio; //Guardamos el caracter
            len++; //Aumentamos la logitud del mismo
        }

        s.inicio++; //Inicio avanza

        if (s.inicio == s.buffer+2*N+1) { //Si inicio llego al EOF de B
            s.inicio = s.buffer; //Lo mandamos al inicio de A
        }
    }

    lexema = realloc(lexema, len*sizeof(char)); //Reajustamos la memoria reservada
    lexema[len] = '\0'; //Volvemos ajustar el \0
    if (len > N) { //Si nos da un lexema de mayor tamaño al permitido pero que aun asi se pudo leer mandamos un error pero continuamos
        tamanho_de_lexema_max_superado();
    }
    return lexema; //Devolvemos el lexema
}

/**
 * Ajusta la posicion de inicio para que sea igual a la de delantero
 */
void ajustar_inicio(){
    s.inicio = s.delantero;
}

/**
 * Finaliza el sistema de entrada
 */
void finalizar_SE() {
    free(s.buffer);
    fclose(arq); //Cierra el archivo
}

