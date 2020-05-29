#ifndef _TOKEN_H__INCLUDED_
#define _TOKEN_H__INCLUDED_

typedef enum _tkCode {
    ID, END, //0,1
    /*cuvinte cheie*/
    BREAK, CHAR, DOUBLE, ELSE,//2-5
    FOR, IF, INT, RETURN, STRUCT, VOID, WHILE, //6-12
    /*constante*/
    CT_INT, CT_REAL, CT_CHAR, CT_STRING, //13-16
    /*delimitator*/
    COMMA, SEMICOLON, LPAR, RPAR, LBRACKET, //16-22
    RBRACKET, LACC, RACC,
    /*operatori*/
    ADD, SUB, MUL, DIV, DOT, AND, OR, NOT, //23-30
    ASSIGN, EQUAL, NOTEQ, LESS, LESSEQ, GREATER, GREATEREQ //31-37
} TkCode;// codurile AL

typedef struct _Token {
    TkCode code;// codul (numele)
    union {
        char* text;// folosit pentru ID, CT_STRING(alocat dinamic)
        long int i;// folosit pentru CT_INT, CT_CHAR
        double r;// folosit pentru CT_REAL
    };
    int line;// linia dinfisierul de intrare
    struct _Token* next;// inlantuire la urmatorul AL
}Token;


//inceputul listei de token-uri, instantiata in alex.c
extern Token* tokens;

#define getTockensList() tokens

//partea de generare, implementata in alex.c
extern Token* addTk(int code);

//partea de gestionare a eroriilor, implementata in error.c
extern void err(const char* fmt, ...);
extern void tkerr(const Token* tk, const char* fmt, ...);

//partea de debugger - implementata in debuglib.c
extern void printToken(Token*);
extern void printAllTokens();
extern void freeList(Token*);

#define SAFEALLOC(var,Type) \
if((var=(Type*)malloc(sizeof(Type)))==NULL){\
    err("not enough memory");\
    exit(10);\
}
#endif