#include "alex.h"
#include <stdio.h>
#include <stdlib.h>

static char* getTkName(Token* tk) {
    static char* tkNames[] = {
    "ID", "END",
    /*cuvinte cheie*/
    "BREAK", "CHAR", "DOUBLE", "ELSE", "FOR", "IF",
        "INT", "RETURN", "STRUCT", "VOID", "WHILE",
        /*constante*/
        "CT_INT", "CT_REAL", "CT_CHAR", "CT_STRING",
        /*delimitator*/
        "COMMA", "SEMICOLON", "LPAR", "RPAR", "LBRACKET", "RBRACKET", "LACC", "RACC",
        /*operatori*/
        "ADD", "SUB", "MUL", "DIV", "DOT", "AND", "OR", "NOT",
            "ASSIGN", "EQUAL", "NOTEQ", "LESS", "LESSEQ", "GREATER", "GREATEREQ"
    };
    return tkNames[tk->code];
}

void printToken(Token* tk) {
    printf("%d %s", tk->line, getTkName(tk));

    if (tk->code == ID || tk->code == CT_STRING) {
        printf(" : %s", tk->text);
    }
    else if (tk->code == tk->code == CT_CHAR) {
        printf(" : %c", (char)tk->i);
    }
    else if (tk->code == CT_INT) {
        printf(" : %d", tk->i);
    }
    else if (tk->code == CT_REAL) {
        printf(" : %lf", tk->r);
    }
    putc('\n', stdout);
}

void printAllTokens() {
    Token *tmpTk;
    for (tmpTk = tokens; tmpTk; tmpTk = tmpTk->next) {
        printToken(tmpTk);
    }
}

void freeList(Token* tk) {
    Token* next;
    while (tk) {
        next = tk->next; // striu ca tk nu e null, din while
        if (tk->code == ID || tk->code == CT_STRING) {
            free(tk->text);
        }
        free(tk);
        tk = next;
    }
}