#include "Al.h"
#include "TS.h"
#include "SE.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "errores.h"

//Implementacion del analizador lexico

//Palabras reservadas junto con su componente lexico
Keyword keywords[] = {
    {"break", BREAK}, {"default", DEFAULT}, {"func", FUNC},
    {"interface", INTERFACE}, {"select", SELECT}, {"case", CASE},
    {"defer", DEFER}, {"go", GO}, {"map", MAP},
    {"struct", STRUCT}, {"chan", CHAN}, {"else", ELSE},
    {"goto", GOTO}, {"package", PACKAGE}, {"switch", SWITCH},
    {"const", CONST}, {"fallthrough", FALLTHROUGH}, {"if", IF},
    {"range", RANGE}, {"type", TYPE}, {"continue", CONTINUE},
    {"for", FOR}, {"import", IMPORT}, {"return", RETURN},
    {"var", VAR}
};


int semicolon = 0; //Variable global usada para medir si es necesario poner el ;

int identificador = 0; //Variable global usada para ver si es necesario acudir a la tabla de simbolos

/**
 * Inicializa el analizador lexico, inicializa el sistema de entrada
 */
void inicializar_AL() {
    inicializar_SE(); //Inicializa el analizador sintactico
}

/**
 * Finaliza el analizador lexico, finaliza el sistema de entrada
 */
void finalizar_AL() {
    finalizar_SE(); //Finaliza el sistema de entrada
}

/**
 * Busca el componente lexico de un lexema en la tabla de simbolos
 * @param lexema lexema a buscar en la tabla
 * @return componente lexico del lexema
 */
int __buscar_TS(char *lexema) {
    return buscar_elemento(lexema); // Buscar en la tabla de simbolos
}

/**
 * Automata para leer los comentarios de una linea
 * @param caracter_actual caracter leido
 */
void __automata_comentarios_una_linea(char caracter_actual) {
    while (caracter_actual != '\n') {
        //Mientras no encuentre un \n avanza
        caracter_actual = (char) leer_caracter();
        ajustar_inicio(); //Mueve el puntero inicio
        if (caracter_actual == '\377') {
            return;
        }
    }
}

/**
 * Automata para leer los comentarios de varias lineas
 * @param caracter_actual caracter leido
 */
void __automata_comentarios_varias_lineas(char caracter_actual) {
    int estado = 2;
    while (estado != 4) {
        caracter_actual = (char) leer_caracter();
        ajustar_inicio();
        if (caracter_actual == '\377') {
            return;
        }
        switch (estado) {
            case 2:
                if (caracter_actual == '*') {
                    estado = 3;
                }
                break;
            case 3:
                if (caracter_actual == '/') {
                    estado = 4;
                } else {
                    estado = 2;
                }
                break;
        }
    }
}

/**
 * Comprueba si un caracter es un operador
 * @param c caracter
 * @return 1 si es,0 si no
 */
int __is_operador(char c) {
    if (c == '+' || c == '&' || c == '|' || c == '*' || c == '^' || c == '/' || c == '%' ||
        c == '<' || c == '>' || c == '=' || c == '!' || c == '(' || c == ')' || c == '-' ||
        c == '[' || c == ']' || c == '{' || c == '}' || c == ',' || c == ':' || c == '~') {
        return 1;
    }
    return 0;
}

/**
 * Automata para leer flotantes, estado final 5
 * @param caracter_actual caracter leido
 * @param estado estado donde inicia la ejecucion
 * @return componente lexico
 */
int __automata_flotantes(char caracter_actual, int estado) {
    while (estado != 5) {
        caracter_actual = (char) leer_caracter();
        if (caracter_actual == '\377') {
            //Si se pilla el final del archivo se devuelve lo que se lleva de lexema
            return FLOTANTE;
        }
        switch (estado) {
            case 1:
                if (isdigit(caracter_actual)) {
                    continue;
                } else if (caracter_actual == 'i') {
                    //Se pilla un numero imaginario
                    semicolon = 1; //Se solicita un ;
                    return IMAGINARIO;
                } else if (caracter_actual == '\n' || caracter_actual == ' ') {
                    semicolon = 1; //Se solicita un ;
                    retroceder_caracter(); //Se retrocede para no perder el caracter leido
                    estado = 5;
                } else if (caracter_actual == 'e' || caracter_actual == 'E') {
                    estado = 2;
                } else if (__is_operador(caracter_actual)) {
                    if (caracter_actual == ')' || caracter_actual == '}' || caracter_actual == ']') {
                        semicolon = 1; //Se solicita un ; para estos operadores
                    }
                    retroceder_caracter(); //Se retrocede para no perder el caracter leido
                    estado = 5;
                } else {
                    estado = 6;
                }
                break;
            case 2:
                if (caracter_actual == '+' || caracter_actual == '-') {
                    estado = 3;
                } else if (isdigit(caracter_actual)) {
                    estado = 4;
                } else {
                    estado = 6;
                }
                break;
            case 3:
                if (isdigit(caracter_actual)) {
                    estado = 4;
                } else {
                    estado = 6;
                }
                break;
            case 4: if (isdigit(caracter_actual)) {
                    continue;
                } else if (__is_operador(caracter_actual)) {
                    if (caracter_actual == ')' || caracter_actual == '}' || caracter_actual == ']') {
                        semicolon = 1; //Se solicita un ; para estos operadores
                    }
                    retroceder_caracter(); //Se retrocede para no perder el caracter leido
                    estado = 5;
                } else if (caracter_actual == 'i') {
                    //Se pilla un numero imaginario
                    semicolon = 1; //Se solicita un ;
                    return IMAGINARIO;
                } else if (caracter_actual == '\n' || caracter_actual == ' ') {
                    semicolon = 1; //Se solicita un ;
                    retroceder_caracter(); //Se retrocede para no perder el caracter leido
                    estado = 5;
                } else {
                    estado = 6;
                }
                break;
            case 6:
                error_lexico();
                estado = 5;
                break;
        }
    }
    return FLOTANTE;
}

/**
 * Automata para leer enteros, estaado final 3
 * @param caracter_actual caracter leido
 * @param estado estado donde inicia la ejecucion
 * @return componente lexico
 */
int __automata_enteros(char caracter_actual, int estado) {
    while (estado != 3) {
        caracter_actual = (char) leer_caracter();
        if (caracter_actual == '\377') {
            return ENTERO; //Si se pilla el final del archivo se devuelve lo que se lleva de lexema
        }
        switch (estado) {
            case 1:
                if (isdigit(caracter_actual)) {
                    continue;
                } else if (caracter_actual == '.') {
                    return __automata_flotantes(caracter_actual, 1);
                } else if (caracter_actual == 'e' || caracter_actual == 'E') {
                    return __automata_flotantes(caracter_actual, 2);
                } else if (caracter_actual == '\n' || caracter_actual == ' ') {
                    semicolon = 1; //Se solicita un ;
                    retroceder_caracter(); //Se retrocede para no perder el caracter leido
                    estado = 3;
                } else if (__is_operador(caracter_actual) || !isdigit(caracter_actual)) {
                    if (caracter_actual == ')' || caracter_actual == '}' || caracter_actual == ']') {
                        semicolon = 1; //Se solicita un ;
                    }
                    retroceder_caracter(); //Se retrocede para no perder el caracter leido
                    estado = 3;
                } else {
                    estado = 2;
                }
                break;
            case 2:
                error_lexico();
                estado = 3;
                break;
        }
    }
    return ENTERO;
}

/**
 * Comprueba si un caracter es hexadecimal
 * @param c caracter
 * @return 1 si es verdad, 0 si no
 */
int __esHexadecimal(char c) {
    return (isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

/**
 * Automata para leer hexadecimales, esatdo final 6
 * @param caracter_actual caracter leido
 * @return componente lexico
 */
int __automata_hexadecimales(char caracter_actual) {
    int estado = 1;
    while (estado != 6) {
        caracter_actual = (char) leer_caracter();
        if (caracter_actual == '\377') {
            return HEXADECIMAL; //Si se pilla el final del archivo se devuelve lo que se lleva de lexema
        }
        switch (estado) {
            case 1:
                if (isdigit(caracter_actual)) {
                    return __automata_enteros(caracter_actual, 1);
                } else if (caracter_actual == 'x' || caracter_actual == 'X') {
                    estado = 2;
                } else if (__is_operador(caracter_actual)) {
                    retroceder_caracter(); //Se retrocede para no perder el caracter leido
                    if (caracter_actual == ')' || caracter_actual == '}' || caracter_actual == ']') {
                        semicolon = 1; //Se solicita un ;
                    }
                    return __automata_enteros(caracter_actual, 3);
                } else if (caracter_actual == '\n' || caracter_actual == ' ') {
                    semicolon = 1; //Se solicita un ;
                    retroceder_caracter(); //Se retrocede para no perder el caracter leido
                    return __automata_enteros(caracter_actual, 3);
                } else if (caracter_actual == 'i') {
                    //Se pilla un numero imaginario
                    return IMAGINARIO;
                } else {
                    estado = 5;
                }
                break;
            case 2:
                if (__esHexadecimal(caracter_actual)) {
                    estado = 3;
                } else if (caracter_actual == '_') {
                    estado = 4;
                } else {
                    estado = 5;
                }
                break;
            case 3:
                if (__esHexadecimal(caracter_actual)) {
                    continue;
                } else if (__is_operador(caracter_actual)) {
                    if (caracter_actual == ')' || caracter_actual == '}' || caracter_actual == ']') {
                        semicolon = 1; //Se solicita un ;
                    }
                    retroceder_caracter(); //Se retrocede para no perder el caracter leido
                    estado = 6;
                } else if (caracter_actual == '\n' || caracter_actual == ' ') {
                    semicolon = 1; //Se solicita un ;
                    retroceder_caracter(); //Se retrocede para no perder el caracter leido
                    estado = 6;
                } else {
                    estado = 5;
                }
                break;
            case 4:
                if (__esHexadecimal(caracter_actual)) {
                    estado = 3;
                } else {
                    estado = 5;
                }
                break;
            case 5:
                error_lexico();
                estado = 6;
                break;
        }
    }
    return HEXADECIMAL;
}

/**
 * Automata para leer operadores que empiezan por '+'
 * @param caracter_actual caracter leido
 * @return struct con el lexema y componente lexico
 */
TIPOELEMENTO __automata_mas(char caracter_actual) {
    TIPOELEMENTO componente;
    caracter_actual = (char) leer_caracter();
    if (caracter_actual == '\377') { //Si se pilla el EOF se devuelve el primer caracter
        componente.nombre = "+";
        componente.token = '+';
        return componente;
    }
    switch (caracter_actual) {
        case '=':
            componente.nombre = "+=";
            componente.token = MASIGUAL;
            break;
        case '+':
            semicolon = 1; //Se solicita un ;
            componente.nombre = "++";
            componente.token = MASMAS;
            break;
        default:
            if (caracter_actual != ' ') {
                // El único elemento que no nos interesa no saltarnos
                retroceder_caracter(); //Se retrocede para no perder el caracter leido
            }
            componente.nombre = "+";
            componente.token = '+';
            break;
    }
    return componente;
}

/**
 * Automata para leer operadores que empiezan por '&'
 * @param caracter_actual caracter leido
 * @return struct con el lexema y componente lexico
 */
TIPOELEMENTO __automata_ampersand(char caracter_actual) {
    TIPOELEMENTO componente;
    caracter_actual = (char) leer_caracter();
    if (caracter_actual == '\377') { //Si se pilla el EOF se devuelve el primer caracter
        componente.nombre = "&";
        componente.token = '&';
        return componente;
    }
    switch (caracter_actual) {
        case '=':
            componente.nombre = "&=";
            componente.token = AMPERSANDIGUAL;
            break;
        case '&':
            componente.nombre = "&&";
            componente.token = AMPERSANDAMPERSAND;
            break;
        case '^':
            caracter_actual = leer_caracter();
            if (caracter_actual == '\377') {
                componente.nombre = "EOF";
                componente.token = EOF;
                return componente;
            }
            if (caracter_actual == '=') {
                componente.nombre = "&^=";
                componente.token = AMPERSANDCIRCUNFLEJOIGUAL;
                break;
            }
            if (caracter_actual != ' ') {
                retroceder_caracter(); //Se retrocede para no perder el caracter leido
            }
            componente.nombre = "&^";
            componente.token = AMPERSANDCIRCUNFLEJO;
            break;
        default:
            if (caracter_actual != ' ') {
                // El único elemento que no nos interesa no saltarnos
                retroceder_caracter(); //Se retrocede para no perder el caracter leido
            }
            componente.nombre = "&";
            componente.token = '&';
            break;
    }
    return componente;
}

/**
 * Automata para leer operadores que empiezan por '='
 * @param caracter_actual caracter leido
 * @return struct con el lexema y componente lexico
 */
TIPOELEMENTO __automata_igual(char caracter_actual) {
    TIPOELEMENTO componente;
    caracter_actual = (char) leer_caracter();
    if (caracter_actual == '\377') { //Si se pilla el EOF se devuelve el primer caracter
        componente.nombre = "=";
        componente.token = '=';
        return componente;
    }
    switch (caracter_actual) {
        case '=':
            componente.nombre = "==";
            componente.token = IGUALIGUAL;
            break;
        default:
            if (caracter_actual != ' ') {
                // El único elemento que no nos interesa saltarnos
                retroceder_caracter(); //Se retrocede para no perder el caracter leido
            }
            componente.nombre = "=";
            componente.token = '=';
            break;
    }
    return componente;
}

/**
 * Automata para leer operadores que empiezan por '-'
 * @param caracter_actual caracter leido
 * @return struct con el lexema y componente lexico
 */
TIPOELEMENTO __automata_menos(char caracter_actual) {
    TIPOELEMENTO componente;
    caracter_actual = (char) leer_caracter();
    if (caracter_actual == '\377') { //Si se pilla el EOF se devuelve el primer caracter
        componente.nombre = "-";
        componente.token = '-';
        return componente;
    }
    switch (caracter_actual) {
        case '=':
            componente.nombre = "-=";
            componente.token = MENOSIGUAL;
            break;
        case '-':
            semicolon = 1; //Se solicita un ;
            componente.nombre = "--";
            componente.token = MENOSMENOS;
            break;
        default:
            if (caracter_actual != ' ') {
                // El único elemento que no nos interesa no saltarnos
                retroceder_caracter(); //Se retrocede para no perder el caracter leido
            }
            componente.nombre = "-";
            componente.token = '-';
            break;
    }
    return componente;
}

/**
 * Automata para leer operadores que empiezan por '|'
 * @param caracter_actual caracter leido
 * @return struct con el lexema y componente lexico
 */
TIPOELEMENTO __automata_barra_horizontal(char caracter_actual) {
    TIPOELEMENTO componente;
    caracter_actual = (char) leer_caracter();
    if (caracter_actual == '\377') { //Si se pilla el EOF se devuelve el primer caracter
        componente.nombre = "|";
        componente.token = '|';
        return componente;
    }
    switch (caracter_actual) {
        case '=':
            componente.nombre = "|=";
            componente.token = BARRAHIGUAL;
            break;
        case '|':
            componente.nombre = "||";
            componente.token = BARRAHBARRAH;
            break;
        default:
            if (caracter_actual != ' ') {
                // El único elemento que no nos interesa saltarnos
                retroceder_caracter(); //Se retrocede para no perder el caracter leido
            }
            componente.nombre = "|";
            componente.token = '|';
            break;
    }
    return componente;
}

/**
 * Automata para leer operadores que empiezan por '<'
 * @param caracter_actual caracter leido
 * @return struct con el lexema y componente lexico
 */
TIPOELEMENTO __automata_menor(char caracter_actual) {
    TIPOELEMENTO componente;
    caracter_actual = (char) leer_caracter();
    if (caracter_actual == '\377') {//Si se pilla el EOF se devuelve el primer caracter
        componente.nombre = "<";
        componente.token = '<';
        return componente;
    }
    switch (caracter_actual) {
        case '=':
            componente.nombre = "<=";
            componente.token = MENORIGUAL;
            break;
        case '-':
            componente.nombre = "<-";
            componente.token = FLECHA;
            break;
        case '<':
            caracter_actual = leer_caracter();
            if (caracter_actual == '\377') {
                componente.nombre = "EOF";
                componente.token = EOF;
                return componente;
            }
            if (caracter_actual == '=') {
                componente.nombre = "<<=";
                componente.token = MENORMENORIGUAL;
                break;
            }
            if (caracter_actual != ' ') {
                retroceder_caracter(); //Se retrocede para no perder el caracter leido
            }
            componente.nombre = "<<";
            componente.token = MENORMENOR;
            break;
        default:
            if (caracter_actual != ' ') {
                // El único elemento que no nos interesa saltarnos
                retroceder_caracter(); //Se retrocede para no perder el caracter leido
            }
            componente.nombre = "<";
            componente.token = '<';
            break;
    }
    return componente;
}

/**
 * Automata para leer operadores que empiezan por '>'
 * @param caracter_actual caracter leido
 * @return struct con el lexema y componente lexico
 */
TIPOELEMENTO __automata_mayor(char caracter_actual) {
    TIPOELEMENTO componente;
    caracter_actual = (char) leer_caracter();
    if (caracter_actual == '\377') { //Si se pilla el EOF se devuelve el primer caracter
        componente.nombre = ">";
        componente.token = '>';
        return componente;
    }
    switch (caracter_actual) {
        case '=':
            componente.nombre = ">=";
            componente.token = MAYORIGUAL;
            break;
        case '<':
            caracter_actual = leer_caracter();
            if (caracter_actual == '\377') {
                componente.nombre = "EOF";
                componente.token = EOF;
                return componente;
            }
            if (caracter_actual == '=') {
                componente.nombre = ">>=";
                componente.token = MAYORMAYORIGUAL;
                break;
            }
            if (caracter_actual != ' ') {
                retroceder_caracter(); //Se retrocede para no perder el caracter leido
            }
            componente.nombre = ">>";
            componente.token = MAYORMAYOR;
            break;
        default:
            if (caracter_actual != ' ') {
                // El único elemento que no nos interesa saltarnos
                retroceder_caracter();//Se retrocede para no perder el caracter leido
            }
            componente.nombre = ">";
            componente.token = '>';
            break;
    }
    return componente;
}

/**
 * Automata para leer operadores que empiezan por '/'
 * @param caracter_actual caracter leido
 * @return struct con el lexema y componente lexico
 */
TIPOELEMENTO __automata_barra(char caracter_actual) {
    TIPOELEMENTO componente;
    caracter_actual = (char) leer_caracter();
    if (caracter_actual == '\377') { //Si se pilla el EOF se devuelve el primer caracter
        componente.nombre = "/";
        componente.token = '/';
        return componente;
    }
    switch (caracter_actual) {
        case '=':
            componente.nombre = "/=";
            componente.token = BARRAIGUAL;
            break;
        default:
            if (caracter_actual != ' ') {
                // El único elemento que no nos interesa saltarnos
                retroceder_caracter(); //Se retrocede para no perder el caracter leido
            }
            componente.nombre = "/";
            componente.token = '/';
            break;
    }
    return componente;
}

/**
 * Automata para leer operadores que empiezan por '*'
 * @param caracter_actual caracter leido
 * @return struct con el lexema y componente lexico
 */
TIPOELEMENTO __automata_asterisco(char caracter_actual) {
    TIPOELEMENTO componente;
    caracter_actual = (char) leer_caracter();
    if (caracter_actual == '\377') { //Si se pilla el EOF se devuelve el primer caracter
        componente.nombre = "*";
        componente.token = '*';
        return componente;
    }
    switch (caracter_actual) {
        case '=':
            componente.nombre = "*=";
            componente.token = ASTERISCOIGUAL;
            break;
        default:
            if (caracter_actual != ' ') {
                // El único elemento que no nos interesa saltarnos
                retroceder_caracter(); //Se retrocede para no perder el caracter leido
            }
            componente.nombre = "*";
            componente.token = '*';
            break;
    }
    return componente;
}

/**
 * Automata para leer operadores que empiezan por '^'
 * @param caracter_actual caracter leido
 * @return struct con el lexema y componente lexico
 */
TIPOELEMENTO __automata_circunflejo(char caracter_actual) {
    TIPOELEMENTO componente;
    caracter_actual = (char) leer_caracter();
    if (caracter_actual == '\377') { //Si se pilla el EOF se devuelve el primer caracter
        componente.nombre = "^";
        componente.token = '^';
        return componente;
    }
    switch (caracter_actual) {
        case '=':
            componente.nombre = "^=";
            componente.token = CIRCUNFLEJOIGUAL;
            break;
        default:
            if (caracter_actual != ' ') {
                // El único elemento que no nos interesa saltarnos
                retroceder_caracter(); //Se retrocede para no perder el caracter leido
            }
            componente.nombre = "^";
            componente.token = '^';
            break;
    }
    return componente;
}

/**
 * Automata para leer operadores que empiezan por '%'
 * @param caracter_actual caracter leido
 * @return struct con el lexema y componente lexico
 */
TIPOELEMENTO __automata_porcentaje(char caracter_actual) {
    TIPOELEMENTO componente;
    caracter_actual = (char) leer_caracter();
    if (caracter_actual == '\377') { //Si se pilla el EOF se devuelve el primer caracter
        componente.nombre = "%";
        componente.token = '%';
        return componente;
    }
    switch (caracter_actual) {
        case '=':
            componente.nombre = "%=";
            componente.token = PORCENTAJEIGUAL;
            break;
        default:
            if (caracter_actual != ' ') {
                // El único elemento que no nos interesa saltarnos
                retroceder_caracter(); //Se retrocede para no perder el caracter leido
            }
            componente.nombre = "%";
            componente.token = '%';
            break;
    }
    return componente;
}

/**
 * Automata para leer operadores que empiezan por ':'
 * @param caracter_actual caracter leido
 * @return struct con el lexema y componente lexico
 */
TIPOELEMENTO __automata_dos_puntos(char caracter_actual) {
    TIPOELEMENTO componente;
    caracter_actual = (char) leer_caracter();
    if (caracter_actual == '\377') { //Si se pilla el EOF se devuelve el primer caracter
        componente.nombre = ":";
        componente.token = ':';
        return componente;
    }
    switch (caracter_actual) {
        case '=':
            componente.nombre = ":=";
            componente.token = DOSPUNTOSIGUAL;
            break;
        default:
            if (caracter_actual != ' ') {
                // El único elemento que no nos interesa saltarnos
                retroceder_caracter(); //Se retrocede para no perder el caracter leido
            }
            componente.nombre = ":";
            componente.token = ':';
            break;
    }
    return componente;
}

/**
 * Automata para cadenas
 * @param caracter_actual caracter leido
 * @return componente lexico
 */
int __automata_cadenas(char caracter_actual) {
    int estado = 1;
    while (estado != 4) {
        caracter_actual = (char) leer_caracter();
        if (caracter_actual == '\377') {
            break;
        }
        switch (estado) {
            case 1:
                if (caracter_actual == '\\') {
                    estado = 3;
                } else if (caracter_actual == '"') {
                    estado = 2;
                }
                break;
            case 2:
                if (caracter_actual == '\n' || caracter_actual == ' ') {
                    semicolon = 1; //Se solicita un ;
                    retroceder_caracter(); //Se retrocede para no perder el caracter leido
                    estado = 4;
                } else if (!isalnum(caracter_actual)) {
                    if (caracter_actual == ')' || caracter_actual == '}' || caracter_actual == ']') {
                        semicolon = 1; //Se solicita un ;
                    }
                    retroceder_caracter(); //Se retrocede para no perder el caracter leido
                    estado = 4;
                }
                break;
            case 3:
                if (caracter_actual == '"') {
                    estado = 1;
                } else {
                    estado = 5;
                }
                break;
            case 5:
                error_lexico();
                estado = 4;
        }
    }
    return CADENA;
}

/**
 * Automata para identificadores
 * @param caracter_actual caracter leido
 */
void __automata_identificador(char caracter_actual) {
    while (1) {
        caracter_actual = (char) leer_caracter();
        if (isalpha(caracter_actual) || isdigit(caracter_actual) || caracter_actual == '_') {
            continue;
        } else if (caracter_actual == '\n' || caracter_actual == ' ') {
            semicolon = 1; //Se solicita un ;
            retroceder_caracter(); //Se retrocede para no perder el caracter leido
            break;
        } else if (caracter_actual == '\377') {
            break;
        }
        if (!isalpha(caracter_actual)) {
            if (caracter_actual == ')' || caracter_actual == '}' || caracter_actual == ']') {
                semicolon = 1; //Se solicita un ;
            }
            retroceder_caracter(); //Se retrocede para no perder el caracter leido
            break;
        }
    }
}

/**
 * Retorna el siguiente componente lexico
 * @return siguiente componente lexico
 */
TIPOELEMENTO sig_componente() {
    TIPOELEMENTO componente;
    char caracter_actual = leer_caracter(); //Lee un caracter

    if (semicolon == 1) {
        //Si se habia solicitado un posible ; en el anterior componente
        while (caracter_actual == ' ' || caracter_actual == '\t') {
            //Leemos espacios de forma indefinida
            ajustar_inicio(); //Mueve inicio
            caracter_actual = leer_caracter();
        }
        if ((caracter_actual == ')' || caracter_actual == '}' || caracter_actual == ']')) {
            //Enontramos un ) } o ]
            ajustar_inicio(); //Mueve inicio
            componente.nombre = "caracter";
            componente.token = (int) caracter_actual;
            return componente; //Se devuelve el caracter pero se mantiene semicolon a 1
        } else if (caracter_actual == '\n') {
            //Encontramos un \n, se debe devolver ; y poner semicolon a 0
            ajustar_inicio(); //Mueve inicio
            semicolon = 0; //Se pone semicolon a 0
            componente.nombre = "caracter";
            componente.token = ';';
            return componente; //Se devuelve ;
        } else if (caracter_actual == '/') {
            //Encontramos un posible comentario, es necesario poner ; si se habia pedido
            caracter_actual = leer_caracter(); //Leemos siguiente caracter para ver si tenemos un comentario
            if (caracter_actual == '\377') {
                //Si se pilla el EOF, retrocedemos y devolvemos la /, ajustando siempre inicio
                componente.nombre = "caracter";
                componente.token = '/';
                ajustar_inicio();
                return componente;
            }
            if (caracter_actual == '*') {
                //Comentarios de varias lineas, se recorre y se devuelve ;
                __automata_comentarios_varias_lineas(caracter_actual);
                componente.nombre = "caracter";
                componente.token = ';';
                ajustar_inicio();
                semicolon = 0;
                return componente;
            } else {
                if (caracter_actual == '/') {
                    //Comentarios de una linea, se recorre y devuelve ;
                    __automata_comentarios_una_linea(caracter_actual);
                    componente.nombre = "caracter";
                    componente.token = ';';
                    ajustar_inicio();
                    semicolon = 0;
                    return componente;
                } else {
                    //Cualquier otra opcion, posible /= asi que se llama al automata de barra
                    retroceder_caracter(); //Se retrocede para no perder el caracter leido
                    componente = __automata_barra(caracter_actual);
                    ajustar_inicio();
                    return componente;
                }
            }
        } else {
            //Si no es ningun caso particular, se pone semicolon a 0, ya que no sera necesario ponerlo por ahora
            semicolon = 0;
        }
    }

    while (caracter_actual == ' ' || caracter_actual == '\t' || caracter_actual == '\n') {
        //Posteriomente, una vez comprobado el posible ;, se avanza hasta hallar un caracter valido para leer
        //Avanza hasta encontrar un caracter valido
        ajustar_inicio(); //Mueve inicio
        caracter_actual = leer_caracter();
    }

    if (caracter_actual == '\377') {
        //Si se pilla el EOF del final del archivo se devuelve
        componente.nombre = "EOF";
        componente.token = EOF;
        return componente;
    }
    switch (caracter_actual) {
        //En funcion del caracter se va a un automata u otro
        // Para los operadores se devuelve el componente y se ajusta inicio
        case '+':
            componente = __automata_mas(caracter_actual);
            ajustar_inicio();
            return componente;
        case '&':
            componente = __automata_ampersand(caracter_actual);
            ajustar_inicio();
            return componente;
        case '=':
            componente = __automata_igual(caracter_actual);
            ajustar_inicio();
            return componente;
        case '-':
            componente = __automata_menos(caracter_actual);
            ajustar_inicio();
            return componente;
        case '|':
            componente = __automata_barra_horizontal(caracter_actual);
            ajustar_inicio();
            return componente;
        case '>':
            componente = __automata_mayor(caracter_actual);
            ajustar_inicio();
            return componente;
        case '<':
            componente = __automata_menor(caracter_actual);
            ajustar_inicio();
            return componente;
        case '*':
            componente = __automata_asterisco(caracter_actual);
            ajustar_inicio();
            return componente;
        case '^':
            componente = __automata_circunflejo(caracter_actual);
            ajustar_inicio();
            return componente;
        case '%':
            componente = __automata_porcentaje(caracter_actual);
            ajustar_inicio();
            return componente;
        case ':':
            componente = __automata_dos_puntos(caracter_actual);
            ajustar_inicio();
            return componente;
        case '!':
            caracter_actual = leer_caracter();
            if (caracter_actual == '\377') {
                componente.nombre = "EOF";
                componente.token = EOF;
                return componente;
            } else if (caracter_actual == '=') {
                //Si es el distinto
                componente.nombre = "!=";
                componente.token = DISTINTO;
                ajustar_inicio();
                return componente;
            } else {
                //Si no es el distinto se retrocede
                retroceder_caracter();
            }
        case '_': //Identificadores pueden empezar con '_'
            __automata_identificador(caracter_actual);
            identificador = 1;
            break;
        case '0': //Posible número hexadecimal
            componente.token = __automata_hexadecimales(caracter_actual);
            break;
        case '.': //Hay que comprobar si es un numero flotante, se lee el siguiente caracter
            caracter_actual = leer_caracter();
            if (caracter_actual == '\377') {
                componente.nombre = "EOF";
                componente.token = EOF;
                return componente;
            } //Si hay un digito, se manda al automata de flotantes
            if (isdigit(caracter_actual)) {
                componente.token = __automata_flotantes(caracter_actual, 1);
            } else {

                retroceder_caracter(); //Se retrocede para no perder el caracter leido
            }
            break;
        case '"': //Cadenas
            componente.token = __automata_cadenas(caracter_actual);
            break;
        case '/': //Comentarios u operadores con /, es necesario leer el siguiente caracter
            caracter_actual = leer_caracter();
            if (caracter_actual == '\377') {
                componente.nombre = "EOF";
                componente.token = EOF;
                return componente;
            }
            if (caracter_actual == '*') {
                //Para los comentarios de varias lineas
                __automata_comentarios_varias_lineas(caracter_actual);

                //Si no se devuelve vacio
                componente.nombre = "";
                componente.token = 0;

                ajustar_inicio();
                return componente;
            } else {
                if (caracter_actual == '/') {
                    //Para los comentarios de una linea
                    __automata_comentarios_una_linea(caracter_actual);

                    componente.nombre = "";
                    componente.token = 0;

                    ajustar_inicio();
                    return componente;
                } else {
                    //Automata de la /, se retrocede un caracter para no perder informacion
                    retroceder_caracter();
                    componente = __automata_barra(caracter_actual);
                    ajustar_inicio();
                    return componente;
                }
            }
            break;
        default:
            if (isalpha(caracter_actual)) {
                //Identificadores
                // Letras → Identificadores
                __automata_identificador(caracter_actual);
                identificador = 1;
            } else if (isdigit(caracter_actual)) {
                //Numeros
                // Números
                componente.token = __automata_enteros(caracter_actual, 1);
            } else {
                //Caracteres solitarios
                componente.nombre = "caracter";
                componente.token = (int) caracter_actual;
                ajustar_inicio();
                return componente;
            }
    }
    //Obtenemos el lexema del sistema de entrada
    componente.nombre = devolver_lexema();
    if (identificador == 1) {
        //Si es un identificador buscamos el token en la tabla de simbolos
        identificador = 0;
        componente.token = __buscar_TS(componente.nombre);
        return componente;
    } //Si no, el componente ya deberia tener el token y se devuelve
    return componente;
}
