#ifndef TOKENS_H
#define TOKENS_H

/*
 * Token definitions for the Go lexical analyzer.
 *
 * Each lexical category maps to a stable integer code. Single-character
 * tokens (delimiters such as '(', ')', '{', ';', ...) are reported using
 * their raw ASCII value, so the codes below deliberately start at 320 to
 * avoid colliding with that range.
 */

/* A token produced by the scanner: the matched text plus its category. */
typedef struct {
    char *lexeme; /* matched source text (the lexeme) */
    int type;     /* token category, one of the codes below or an ASCII value */
} Token;

/* Keywords (Go reserved words) */
#define KW_BREAK        320
#define KW_DEFAULT      321
#define KW_FUNC         322
#define KW_INTERFACE    323
#define KW_SELECT       324
#define KW_CASE         325
#define KW_DEFER        326
#define KW_GO           327
#define KW_MAP          328
#define KW_STRUCT       329
#define KW_CHAN         330
#define KW_ELSE         331
#define KW_GOTO         332
#define KW_PACKAGE      333
#define KW_SWITCH       334
#define KW_CONST        335
#define KW_FALLTHROUGH  336
#define KW_IF           337
#define KW_RANGE        338
#define KW_TYPE         339
#define KW_CONTINUE     340
#define KW_FOR          341
#define KW_IMPORT       342
#define KW_RETURN       343
#define KW_VAR          344

/* Compound operators (single-character operators use their ASCII value) */
#define OP_PLUS_ASSIGN     350  /* +=  */
#define OP_INCREMENT       351  /* ++  */
#define OP_AND_ASSIGN      352  /* &=  */
#define OP_LOGICAL_AND     353  /* &&  */
#define OP_AND_NOT         354  /* &^  */
#define OP_AND_NOT_ASSIGN  355  /* &^= */
#define OP_EQUAL           356  /* ==  */
#define OP_MINUS_ASSIGN    357  /* -=  */
#define OP_DECREMENT       358  /* --  */
#define OP_OR_ASSIGN       359  /* |=  */
#define OP_LOGICAL_OR      360  /* ||  */
#define OP_LESS_EQUAL      361  /* <=  */
#define OP_ARROW           362  /* <-  */
#define OP_SHL_ASSIGN      363  /* <<= */
#define OP_SHL             364  /* <<  */
#define OP_GREATER_EQUAL   365  /* >=  */
#define OP_SHR_ASSIGN      366  /* >>= */
#define OP_SHR             367  /* >>  */
#define OP_DIV_ASSIGN      368  /* /=  */
#define OP_MUL_ASSIGN      370  /* *=  */
#define OP_XOR_ASSIGN      371  /* ^=  */
#define OP_MOD_ASSIGN      372  /* %=  */
#define OP_ELLIPSIS        373  /* ... (reserved: not scanned yet) */
#define OP_NOT_EQUAL       374  /* !=  */
#define OP_DEFINE          375  /* :=  */

/* Numeric literals */
#define LIT_INT        376
#define LIT_FLOAT      377
#define LIT_HEX        378
#define LIT_IMAGINARY  379

/* String literal and identifier */
#define LIT_STRING     390
#define TOKEN_ID       400

#endif /* TOKENS_H */
