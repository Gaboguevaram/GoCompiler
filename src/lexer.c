#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#include "lexer.h"
#include "input_buffer.h"
#include "symbol_table.h"
#include "errors.h"

/* Implementation of the lexical analyzer. */

/* Go reserved words and their token codes. Preloaded into the symbol table. */
Keyword keywords[] = {
    {"break", KW_BREAK}, {"default", KW_DEFAULT}, {"func", KW_FUNC},
    {"interface", KW_INTERFACE}, {"select", KW_SELECT}, {"case", KW_CASE},
    {"defer", KW_DEFER}, {"go", KW_GO}, {"map", KW_MAP},
    {"struct", KW_STRUCT}, {"chan", KW_CHAN}, {"else", KW_ELSE},
    {"goto", KW_GOTO}, {"package", KW_PACKAGE}, {"switch", KW_SWITCH},
    {"const", KW_CONST}, {"fallthrough", KW_FALLTHROUGH}, {"if", KW_IF},
    {"range", KW_RANGE}, {"type", KW_TYPE}, {"continue", KW_CONTINUE},
    {"for", KW_FOR}, {"import", KW_IMPORT}, {"return", KW_RETURN},
    {"var", KW_VAR}
};

/*
 * Go inserts semicolons automatically at the end of certain tokens. This flag
 * records that the *previous* token could require an inserted ';' so the next
 * call can emit one when it sees a newline (see next_token).
 */
static int need_semicolon = 0;

/* Set while scanning an identifier, so next_token knows to consult the table. */
static int is_identifier = 0;

/**
 * Initialises the lexical analyzer, which initialises the input system.
 * @param path path to the source file to scan.
 */
void lexer_init(const char *path) {
    input_open(path);
}

/**
 * Finalises the lexical analyzer, which finalises the input system.
 */
void lexer_close(void) {
    input_close();
}

/**
 * Checks whether a character is an operator or delimiter.
 * @param c character to check.
 * @return 1 if it is, 0 otherwise.
 */
static int is_operator_char(char c) {
    if (c == '+' || c == '&' || c == '|' || c == '*' || c == '^' || c == '/' || c == '%' ||
        c == '<' || c == '>' || c == '=' || c == '!' || c == '(' || c == ')' || c == '-' ||
        c == '[' || c == ']' || c == '{' || c == '}' || c == ',' || c == ':' || c == '~') {
        return 1;
    }
    return 0;
}

/**
 * Checks whether a character is a hexadecimal digit.
 * @param c character to check.
 * @return 1 if it is, 0 otherwise.
 */
static int is_hex_digit(char c) {
    return (isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

/* ----------------------------- comments ------------------------------ */

/**
 * Automaton that reads single-line comments, up to the newline or end of input.
 * @param current the character just read.
 */
static void scan_line_comment(char current) {
    while (current != '\n') {
        current = read_char();
        sync_lexeme_start(); /* comments are discarded, so advance start */
        if (current == EOF_SENTINEL) {
            return;
        }
    }
}

/**
 * Automaton that reads multi-line comments (slash-star ... star-slash),
 * including the closing delimiter. Final state is 4.
 * @param current the character just read.
 */
static void scan_block_comment(char current) {
    int state = 2;
    while (state != 4) {
        current = read_char();
        sync_lexeme_start();
        if (current == EOF_SENTINEL) {
            return;
        }
        switch (state) {
            case 2:
                if (current == '*') {
                    state = 3;
                }
                break;
            case 3:
                if (current == '/') {
                    state = 4; /* closing delimiter seen */
                } else {
                    state = 2;
                }
                break;
        }
    }
}

/* ----------------------------- numbers ------------------------------- */

/**
 * Automaton that reads floating-point literals. Final state is 5.
 * @param current the character just read.
 * @param state the state in which execution starts (1 after a '.', 2 after an
 *              exponent).
 * @return the token type (LIT_FLOAT or LIT_IMAGINARY).
 */
static int scan_float(char current, int state) {
    while (state != 5) {
        current = read_char();
        if (current == EOF_SENTINEL) {
            return LIT_FLOAT; /* end of file: return what we have */
        }
        switch (state) {
            case 1:
                if (isdigit(current)) {
                    continue;
                } else if (current == 'i') {
                    need_semicolon = 1;
                    return LIT_IMAGINARY;
                } else if (current == '\n' || current == ' ') {
                    need_semicolon = 1;
                    unread_char();
                    state = 5;
                } else if (current == 'e' || current == 'E') {
                    state = 2;
                } else if (is_operator_char(current)) {
                    if (current == ')' || current == '}' || current == ']') {
                        need_semicolon = 1;
                    }
                    unread_char();
                    state = 5;
                } else {
                    state = 6;
                }
                break;
            case 2:
                if (current == '+' || current == '-') {
                    state = 3;
                } else if (isdigit(current)) {
                    state = 4;
                } else {
                    state = 6;
                }
                break;
            case 3:
                if (isdigit(current)) {
                    state = 4;
                } else {
                    state = 6;
                }
                break;
            case 4:
                if (isdigit(current)) {
                    continue;
                } else if (is_operator_char(current)) {
                    if (current == ')' || current == '}' || current == ']') {
                        need_semicolon = 1;
                    }
                    unread_char();
                    state = 5;
                } else if (current == 'i') {
                    need_semicolon = 1;
                    return LIT_IMAGINARY;
                } else if (current == '\n' || current == ' ') {
                    need_semicolon = 1;
                    unread_char();
                    state = 5;
                } else {
                    state = 6;
                }
                break;
            case 6:
                lexical_error();
                state = 5;
                break;
        }
    }
    return LIT_FLOAT;
}

/**
 * Automaton that reads integer literals. Final state is 3.
 * @param current the character just read.
 * @param state the state in which execution starts.
 * @return the token type (LIT_INT, or a float/imaginary type on '.'/'e').
 */
static int scan_integer(char current, int state) {
    while (state != 3) {
        current = read_char();
        if (current == EOF_SENTINEL) {
            return LIT_INT;
        }
        switch (state) {
            case 1:
                if (isdigit(current)) {
                    continue;
                } else if (current == '.') {
                    return scan_float(current, 1);
                } else if (current == 'e' || current == 'E') {
                    return scan_float(current, 2);
                } else if (current == '\n' || current == ' ') {
                    need_semicolon = 1;
                    unread_char();
                    state = 3;
                } else if (is_operator_char(current) || !isdigit(current)) {
                    if (current == ')' || current == '}' || current == ']') {
                        need_semicolon = 1;
                    }
                    unread_char();
                    state = 3;
                } else {
                    state = 2;
                }
                break;
            case 2:
                lexical_error();
                state = 3;
                break;
        }
    }
    return LIT_INT;
}

/**
 * Automaton that reads numbers starting with '0' (hexadecimal "0x...", or a
 * plain integer). Final state is 6.
 * @param current the character just read.
 * @return the token type (LIT_HEX, LIT_INT, or LIT_IMAGINARY).
 */
static int scan_hex(char current) {
    int state = 1;
    while (state != 6) {
        current = read_char();
        if (current == EOF_SENTINEL) {
            return LIT_HEX;
        }
        switch (state) {
            case 1:
                if (isdigit(current)) {
                    return scan_integer(current, 1);
                } else if (current == 'x' || current == 'X') {
                    state = 2;
                } else if (is_operator_char(current)) {
                    unread_char();
                    if (current == ')' || current == '}' || current == ']') {
                        need_semicolon = 1;
                    }
                    return scan_integer(current, 3);
                } else if (current == '\n' || current == ' ') {
                    need_semicolon = 1;
                    unread_char();
                    return scan_integer(current, 3);
                } else if (current == 'i') {
                    return LIT_IMAGINARY;
                } else {
                    state = 5;
                }
                break;
            case 2:
                if (is_hex_digit(current)) {
                    state = 3;
                } else if (current == '_') {
                    state = 4;
                } else {
                    state = 5;
                }
                break;
            case 3:
                if (is_hex_digit(current)) {
                    continue;
                } else if (is_operator_char(current)) {
                    if (current == ')' || current == '}' || current == ']') {
                        need_semicolon = 1;
                    }
                    unread_char();
                    state = 6;
                } else if (current == '\n' || current == ' ') {
                    need_semicolon = 1;
                    unread_char();
                    state = 6;
                } else {
                    state = 5;
                }
                break;
            case 4:
                if (is_hex_digit(current)) {
                    state = 3;
                } else {
                    state = 5;
                }
                break;
            case 5:
                lexical_error();
                state = 6;
                break;
        }
    }
    return LIT_HEX;
}

/* ----------------------------- strings ------------------------------- */

/**
 * Automaton that reads string literals ("..."), handling escaped quotes.
 * Reports an error if end of input is reached before the closing quote.
 * Final state is 4.
 * @param current the character just read.
 * @return the token type (LIT_STRING).
 */
static int scan_string(char current) {
    int state = 1;
    while (state != 4) {
        current = read_char();
        if (current == EOF_SENTINEL) {
            if (state == 1 || state == 3) {
                /* still inside the string (or mid-escape): never closed */
                unterminated_string_error();
            }
            break;
        }
        switch (state) {
            case 1:
                if (current == '\\') {
                    state = 3; /* escape sequence */
                } else if (current == '"') {
                    state = 2; /* possible closing quote */
                }
                break;
            case 2:
                if (current == '\n' || current == ' ') {
                    need_semicolon = 1;
                    unread_char();
                    state = 4;
                } else if (!isalnum(current)) {
                    if (current == ')' || current == '}' || current == ']') {
                        need_semicolon = 1;
                    }
                    unread_char();
                    state = 4;
                }
                break;
            case 3:
                if (current == '"') {
                    state = 1; /* escaped quote: back inside the string */
                } else {
                    state = 5;
                }
                break;
            case 5:
                lexical_error();
                state = 4;
        }
    }
    return LIT_STRING;
}

/* --------------------------- identifiers ----------------------------- */

/**
 * Automaton that reads the rest of an identifier ([A-Za-z0-9_]*).
 * @param current the character just read.
 */
static void scan_identifier(char current) {
    while (1) {
        current = read_char();
        if (isalpha(current) || isdigit(current) || current == '_') {
            continue;
        } else if (current == '\n' || current == ' ') {
            need_semicolon = 1;
            unread_char();
            break;
        } else if (current == EOF_SENTINEL) {
            break;
        }
        if (!isalpha(current)) {
            if (current == ')' || current == '}' || current == ']') {
                need_semicolon = 1;
            }
            unread_char();
            break;
        }
    }
}

/* ----------------------------- operators ----------------------------- */

/**
 * Automaton that reads operators starting with '+': '+', '+=', '++'.
 * @return struct with the lexeme and the token type.
 */
static Token scan_plus(void) {
    Token t;
    char c = read_char();
    if (c == EOF_SENTINEL) { /* EOF: emit the single-character operator */
        t.lexeme = "+";
        t.type = '+';
        return t;
    }
    switch (c) {
        case '=':
            t.lexeme = "+=";
            t.type = OP_PLUS_ASSIGN;
            break;
        case '+':
            need_semicolon = 1; /* '++' can end a statement */
            t.lexeme = "++";
            t.type = OP_INCREMENT;
            break;
        default:
            if (c != ' ') {
                unread_char(); /* push back the lookahead character */
            }
            t.lexeme = "+";
            t.type = '+';
            break;
    }
    return t;
}

/**
 * Automaton that reads operators starting with '-': '-', '-=', '--'.
 * @return struct with the lexeme and the token type.
 */
static Token scan_minus(void) {
    Token t;
    char c = read_char();
    if (c == EOF_SENTINEL) {
        t.lexeme = "-";
        t.type = '-';
        return t;
    }
    switch (c) {
        case '=':
            t.lexeme = "-=";
            t.type = OP_MINUS_ASSIGN;
            break;
        case '-':
            need_semicolon = 1; /* '--' can end a statement */
            t.lexeme = "--";
            t.type = OP_DECREMENT;
            break;
        default:
            if (c != ' ') {
                unread_char();
            }
            t.lexeme = "-";
            t.type = '-';
            break;
    }
    return t;
}

/**
 * Automaton that reads operators starting with '|': '|', '|=', '||'.
 * @return struct with the lexeme and the token type.
 */
static Token scan_pipe(void) {
    Token t;
    char c = read_char();
    if (c == EOF_SENTINEL) {
        t.lexeme = "|";
        t.type = '|';
        return t;
    }
    switch (c) {
        case '=':
            t.lexeme = "|=";
            t.type = OP_OR_ASSIGN;
            break;
        case '|': /* '||' does not end a statement, so no semicolon here */
            t.lexeme = "||";
            t.type = OP_LOGICAL_OR;
            break;
        default:
            if (c != ' ') {
                unread_char();
            }
            t.lexeme = "|";
            t.type = '|';
            break;
    }
    return t;
}

/**
 * Automaton that reads operators starting with '=': '=', '=='.
 * @return struct with the lexeme and the token type.
 */
static Token scan_equal(void) {
    Token t;
    char c = read_char();
    if (c == EOF_SENTINEL) {
        t.lexeme = "=";
        t.type = '=';
        return t;
    }
    switch (c) {
        case '=':
            t.lexeme = "==";
            t.type = OP_EQUAL;
            break;
        default:
            if (c != ' ') {
                unread_char();
            }
            t.lexeme = "=";
            t.type = '=';
            break;
    }
    return t;
}

/**
 * Automaton that reads operators starting with '*': '*', '*='.
 * @return struct with the lexeme and the token type.
 */
static Token scan_star(void) {
    Token t;
    char c = read_char();
    if (c == EOF_SENTINEL) {
        t.lexeme = "*";
        t.type = '*';
        return t;
    }
    switch (c) {
        case '=':
            t.lexeme = "*=";
            t.type = OP_MUL_ASSIGN;
            break;
        default:
            if (c != ' ') {
                unread_char();
            }
            t.lexeme = "*";
            t.type = '*';
            break;
    }
    return t;
}

/**
 * Automaton that reads operators starting with '^': '^', '^='.
 * @return struct with the lexeme and the token type.
 */
static Token scan_caret(void) {
    Token t;
    char c = read_char();
    if (c == EOF_SENTINEL) {
        t.lexeme = "^";
        t.type = '^';
        return t;
    }
    switch (c) {
        case '=':
            t.lexeme = "^=";
            t.type = OP_XOR_ASSIGN;
            break;
        default:
            if (c != ' ') {
                unread_char();
            }
            t.lexeme = "^";
            t.type = '^';
            break;
    }
    return t;
}

/**
 * Automaton that reads operators starting with '%': '%', '%='.
 * @return struct with the lexeme and the token type.
 */
static Token scan_percent(void) {
    Token t;
    char c = read_char();
    if (c == EOF_SENTINEL) {
        t.lexeme = "%";
        t.type = '%';
        return t;
    }
    switch (c) {
        case '=':
            t.lexeme = "%=";
            t.type = OP_MOD_ASSIGN;
            break;
        default:
            if (c != ' ') {
                unread_char();
            }
            t.lexeme = "%";
            t.type = '%';
            break;
    }
    return t;
}

/**
 * Automaton that reads operators starting with ':': ':', ':='.
 * @return struct with the lexeme and the token type.
 */
static Token scan_colon(void) {
    Token t;
    char c = read_char();
    if (c == EOF_SENTINEL) {
        t.lexeme = ":";
        t.type = ':';
        return t;
    }
    switch (c) {
        case '=':
            t.lexeme = ":=";
            t.type = OP_DEFINE;
            break;
        default:
            if (c != ' ') {
                unread_char();
            }
            t.lexeme = ":";
            t.type = ':';
            break;
    }
    return t;
}

/**
 * Automaton that reads operators starting with '/': '/', '/='.
 * @return struct with the lexeme and the token type.
 */
static Token scan_slash(void) {
    Token t;
    char c = read_char();
    if (c == EOF_SENTINEL) {
        t.lexeme = "/";
        t.type = '/';
        return t;
    }
    switch (c) {
        case '=':
            t.lexeme = "/=";
            t.type = OP_DIV_ASSIGN;
            break;
        default:
            if (c != ' ') {
                unread_char();
            }
            t.lexeme = "/";
            t.type = '/';
            break;
    }
    return t;
}

/**
 * Automaton that reads operators starting with '&': '&', '&=', '&&', '&^',
 * '&^='.
 * @return struct with the lexeme and the token type.
 */
static Token scan_ampersand(void) {
    Token t;
    char c = read_char();
    if (c == EOF_SENTINEL) {
        t.lexeme = "&";
        t.type = '&';
        return t;
    }
    switch (c) {
        case '=':
            t.lexeme = "&=";
            t.type = OP_AND_ASSIGN;
            break;
        case '&':
            t.lexeme = "&&";
            t.type = OP_LOGICAL_AND;
            break;
        case '^':
            c = read_char();
            if (c == EOF_SENTINEL) {
                t.lexeme = "EOF";
                t.type = EOF;
                return t;
            }
            if (c == '=') {
                t.lexeme = "&^=";
                t.type = OP_AND_NOT_ASSIGN;
                break;
            }
            if (c != ' ') {
                unread_char();
            }
            t.lexeme = "&^";
            t.type = OP_AND_NOT;
            break;
        default:
            if (c != ' ') {
                unread_char();
            }
            t.lexeme = "&";
            t.type = '&';
            break;
    }
    return t;
}

/**
 * Automaton that reads operators starting with '<': '<', '<=', '<-', '<<',
 * '<<='.
 * @return struct with the lexeme and the token type.
 */
static Token scan_less(void) {
    Token t;
    char c = read_char();
    if (c == EOF_SENTINEL) {
        t.lexeme = "<";
        t.type = '<';
        return t;
    }
    switch (c) {
        case '=':
            t.lexeme = "<=";
            t.type = OP_LESS_EQUAL;
            break;
        case '-':
            t.lexeme = "<-";
            t.type = OP_ARROW;
            break;
        case '<':
            c = read_char();
            if (c == EOF_SENTINEL) {
                t.lexeme = "EOF";
                t.type = EOF;
                return t;
            }
            if (c == '=') {
                t.lexeme = "<<=";
                t.type = OP_SHL_ASSIGN;
                break;
            }
            if (c != ' ') {
                unread_char();
            }
            t.lexeme = "<<";
            t.type = OP_SHL;
            break;
        default:
            if (c != ' ') {
                unread_char();
            }
            t.lexeme = "<";
            t.type = '<';
            break;
    }
    return t;
}

/**
 * Automaton that reads operators starting with '>': '>', '>=', '>>', '>>='.
 * @return struct with the lexeme and the token type.
 */
static Token scan_greater(void) {
    Token t;
    char c = read_char();
    if (c == EOF_SENTINEL) {
        t.lexeme = ">";
        t.type = '>';
        return t;
    }
    switch (c) {
        case '=':
            t.lexeme = ">=";
            t.type = OP_GREATER_EQUAL;
            break;
        case '>': /* second '>' of a shift operator */
            c = read_char();
            if (c == EOF_SENTINEL) {
                t.lexeme = "EOF";
                t.type = EOF;
                return t;
            }
            if (c == '=') {
                t.lexeme = ">>=";
                t.type = OP_SHR_ASSIGN;
                break;
            }
            if (c != ' ') {
                unread_char();
            }
            t.lexeme = ">>";
            t.type = OP_SHR;
            break;
        default:
            if (c != ' ') {
                unread_char();
            }
            t.lexeme = ">";
            t.type = '>';
            break;
    }
    return t;
}

/* --------------------------- main dispatch --------------------------- */

/**
 * Returns the next token. Dispatches on the first character to the automaton
 * that handles its category.
 * @return the next token: struct with the lexeme and the token type.
 */
Token next_token(void) {
    Token t;
    char c = read_char();

    /* Go's automatic semicolon insertion: if the previous token may need a
       trailing ';', decide here what to emit before scanning a real token. */
    if (need_semicolon == 1) {
        while (c == ' ' || c == '\t') {
            sync_lexeme_start();
            c = read_char();
        }
        if (c == ')' || c == '}' || c == ']') {
            /* delimiter: emit it but keep need_semicolon set */
            sync_lexeme_start();
            t.lexeme = "char";
            t.type = (int) c;
            return t;
        } else if (c == '\n') {
            /* newline: emit the inserted ';' */
            sync_lexeme_start();
            need_semicolon = 0;
            t.lexeme = "char";
            t.type = ';';
            return t;
        } else if (c == '/') {
            /* a comment still needs the inserted ';' beforehand */
            c = read_char();
            if (c == EOF_SENTINEL) {
                t.lexeme = "char";
                t.type = '/';
                sync_lexeme_start();
                return t;
            }
            if (c == '*') {
                scan_block_comment(c);
                t.lexeme = "char";
                t.type = ';';
                sync_lexeme_start();
                need_semicolon = 0;
                return t;
            } else {
                if (c == '/') {
                    scan_line_comment(c);
                    t.lexeme = "char";
                    t.type = ';';
                    sync_lexeme_start();
                    need_semicolon = 0;
                    return t;
                } else {
                    /* not a comment: a '/' or '/=' operator */
                    unread_char();
                    t = scan_slash();
                    sync_lexeme_start();
                    return t;
                }
            }
        } else {
            need_semicolon = 0;
        }
    }

    /* skip whitespace until a meaningful character */
    while (c == ' ' || c == '\t' || c == '\n') {
        sync_lexeme_start();
        c = read_char();
    }

    if (c == EOF_SENTINEL) {
        t.lexeme = "EOF";
        t.type = EOF;
        return t;
    }

    switch (c) {
        /* operators: scan, sync the lexeme start, and return */
        case '+':
            t = scan_plus();
            sync_lexeme_start();
            return t;
        case '-':
            t = scan_minus();
            sync_lexeme_start();
            return t;
        case '|':
            t = scan_pipe();
            sync_lexeme_start();
            return t;
        case '=':
            t = scan_equal();
            sync_lexeme_start();
            return t;
        case '*':
            t = scan_star();
            sync_lexeme_start();
            return t;
        case '^':
            t = scan_caret();
            sync_lexeme_start();
            return t;
        case '%':
            t = scan_percent();
            sync_lexeme_start();
            return t;
        case ':':
            t = scan_colon();
            sync_lexeme_start();
            return t;
        case '&':
            t = scan_ampersand();
            sync_lexeme_start();
            return t;
        case '<':
            t = scan_less();
            sync_lexeme_start();
            return t;
        case '>':
            t = scan_greater();
            sync_lexeme_start();
            return t;
        case '!':
            c = read_char();
            if (c == EOF_SENTINEL) {
                t.lexeme = "EOF";
                t.type = EOF;
                return t;
            } else if (c == '=') {
                t.lexeme = "!=";
                t.type = OP_NOT_EQUAL;
                sync_lexeme_start();
                return t;
            }
            /* not '!=': emit a standalone '!' token */
            unread_char();
            t.lexeme = "char";
            t.type = '!';
            sync_lexeme_start();
            return t;
        case '_': /* identifiers may start with '_' */
            scan_identifier(c);
            is_identifier = 1;
            break;
        case '0': /* possible hexadecimal number */
            t.type = scan_hex(c);
            break;
        case '.': /* a floating-point literal ".5", otherwise a '.' delimiter */
            c = read_char();
            if (c == EOF_SENTINEL) {
                t.lexeme = "EOF";
                t.type = EOF;
                return t;
            }
            if (isdigit(c)) {
                t.type = scan_float(c, 1);
                break; /* lexeme is fetched after the switch */
            }
            /* not a number: emit a standalone '.' token */
            unread_char();
            t.lexeme = "char";
            t.type = '.';
            sync_lexeme_start();
            return t;
        case '"': /* string literal */
            t.type = scan_string(c);
            break;
        case '/': /* comment or '/' / '/=' operator */
            c = read_char();
            if (c == EOF_SENTINEL) {
                t.lexeme = "EOF";
                t.type = EOF;
                return t;
            }
            if (c == '*') {
                scan_block_comment(c);
                t.lexeme = ""; /* empty lexeme: caller skips comments */
                t.type = 0;
                sync_lexeme_start();
                return t;
            } else {
                if (c == '/') {
                    scan_line_comment(c);
                    t.lexeme = "";
                    t.type = 0;
                    sync_lexeme_start();
                    return t;
                } else {
                    unread_char();
                    t = scan_slash();
                    sync_lexeme_start();
                    return t;
                }
            }
        default:
            if (isalpha(c)) {
                scan_identifier(c);
                is_identifier = 1;
            } else if (isdigit(c)) {
                t.type = scan_integer(c, 1);
            } else {
                /* standalone delimiter: '(', ')', '{', '}', '[', ']', ',', ';' */
                t.lexeme = "char";
                t.type = (int) c;
                sync_lexeme_start();
                return t;
            }
    }

    /* identifiers and numbers reach here with the lexeme still in the buffer */
    t.lexeme = get_lexeme();
    if (is_identifier == 1) {
        is_identifier = 0;
        t.type = symtable_lookup(&t.lexeme); /* may free/replace t.lexeme */
        return t;
    }
    return t;
}
