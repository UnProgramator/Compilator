#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "alex.h"

int line= 1;
Token *lastToken, *tokens;
char* pCrtCh;

int getNextTocken();

Token* AnalizorLexical(char* InputFileBufferPointer) {
    lastToken = NULL;
    pCrtCh = InputFileBufferPointer;
    do {
        getNextTocken();
    } while (lastToken->code != END);
    return tokens;
}

char makeSpecial(char c) {// creaza caractere speciale
    switch (c) {
    case 'a':
        return '\a';
        break;
    case 'b':
        return '\b';
        break;
    case 'f':
        return '\f';
        break;
    case 'n':
        return '\n';
        break;
    case 'r':
        return '\r';
        break;
    case 't':
        return '\t';
        break;
    case 'v':
        return '\v';
        break;
    case '\'':
        return '\'';
        break;
    case '?':
        return '\?';
        break;
    case '\"':
        return '\"';
        break;
    case '\\':
        return  '\\';
        break;
    case '0':
        return '\0';
        break;
    }
    return 255; //cazul "default" la care nu se poate ajunge teoretic
}

char* makeString(const char* start, const char* finish, int canConSpec/*poate contine caracter cu \*/) {
    char* string;//aloc mai mult daca contine caractere seciale
    if ((string = (char*)malloc(sizeof(char) * (finish - start + 1))) == NULL) {
        fprintf(stderr, "Eroare la alocarea dinamica. Spatiu insuficient");
        exit(0);
    }

    if (canConSpec == 0) {
        memcpy(string, start, finish - start);
        string[finish - start] = '\0';
    }
    else {
        int i=0;
        for (; start<finish; start++) {
            if (*start == '\\') {
                start++;
                string[i++] = makeSpecial(*start);
            }
            else string[i++] = *start;
        }
        string[i] = '\0';
    }
    return string;
}

int makeInt(char* sir) {
    if(sir[0] == '0') { //daca incepe cu 0
        if (sir[1] == 'x') 
            return strtol(sir, NULL, 16);
        else 
            return strtol(sir, NULL, 8);
    }
    return strtol(sir, NULL, 10);
}

Token* addTk(int code){
    Token*tk;
    SAFEALLOC(tk,Token)
    tk->code=code;
    tk->line=line;
    tk->next=NULL;
    if(lastToken){
        lastToken->next=tk;
    }else{
        tokens=tk;
    }
    lastToken=tk;
    return tk;
}
    
int getNextTocken(){
    int state=0,nCh;
    char ch;
    char* pStartCh;
    char *tmp;
    Token *tk=NULL;

    while (1) { // bucla infinita
        ch = *pCrtCh;

        switch (state) {
        case 0:
            if (isalpha(ch) || ch == '_') {
                pStartCh = pCrtCh;
                pCrtCh++;
                state = 30;
            }
            else if (isdigit(ch)) {
                pStartCh = pCrtCh;
                pCrtCh++;
                if (ch == '0')
                    state = 35;
                else
                    state = 41;
            } //elif isdigit
            else {
                switch (ch) {
                case ',':
                    pCrtCh++;
                    state = 1;
                    break;
                case ';':
                    pCrtCh++;
                    state = 2;
                    break;
                case '(':
                    pCrtCh++;
                    state = 3;
                    break;
                case ')':
                    pCrtCh++;
                    state = 4;
                    break;
                case '[':
                    pCrtCh++;
                    state = 5;
                    break;
                case ']':
                    pCrtCh++;
                    state = 6;
                    break;
                case '{':
                    pCrtCh++;
                    state = 7;
                    break;
                case '}':
                    pCrtCh++;
                    state = 8;
                    break;
                case '+':
                    pCrtCh++;
                    state = 9;
                    break;
                case '-':
                    pCrtCh++;
                    state = 10;
                    break;
                case '*':
                    pCrtCh++;
                    state = 11;
                    break;
                case '/':
                    pCrtCh++;
                    state = 12;
                    break;
                case '.':
                    pCrtCh++;
                    state = 13;
                    break;
                case '|':
                    pCrtCh++;
                    state = 14;
                    break;
                case '&':
                    pCrtCh++;
                    state = 16;
                    break;
                case '!':
                    pCrtCh++;
                    state = 18;
                    break;
                case '=':
                    pCrtCh++;
                    state = 24;
                    break;
                case '<':
                    pCrtCh++;
                    state = 27;
                    break;
                case '>':
                    pCrtCh++;
                    state = 21;
                    break;

                case '\'':
                    pStartCh = pCrtCh;
                    pCrtCh++;
                    state = 48;
                    break;
                case '\"':
                    pStartCh = pCrtCh;
                    pCrtCh++;
                    state = 51;
                    break;

                case '\0':
                    addTk(END);
                    return END;
                    break;
                case '\n':
                    line++;
                    pCrtCh++;
                    break;
                case ' ': case '\r': case '\t':
                    pCrtCh++;
                    break;
                default:
                    tkerr(addTk(END), "caracter invalid %c", ch);
                }//switch(ch)
            }//else
        //DELIMITATORI + OPERATORI terminale
            break;
        case 1:
            addTk(COMMA);
            return COMMA;
            break;
        case 2:
            addTk(SEMICOLON);
            return SEMICOLON;
            break;
        case 3:
            addTk(LPAR);
            return LPAR;
            break;
        case 4:
            addTk(RPAR);
            return RPAR;
            break;
        case 5:
            addTk(LBRACKET);
            return LBRACKET;
            break;
        case 6:
            addTk(RBRACKET);
            return RBRACKET;
            break;
        case 7:
            addTk(LACC);
            return LACC;
            break;
        case 8:
            addTk(RACC);
            return RACC;
            break;
        case 9:
            addTk(ADD);
            return ADD;
            break;
        case 10:
            addTk(SUB);
            return SUB;
            break;
        case 11:
            addTk(MUL);
            return MUL;
            break;
        case 54:
            addTk(DIV);
            return DIV;
            break;
        case 13:
            addTk(DOT);
            return DOT;
            break;
        case 15:
            addTk(OR);
            return OR;
            break;
        case 17:
            addTk(AND);
            return AND;
            break;
        case 19:
            addTk(NOT);
            return NOT;
            break;
        case 20:
            addTk(NOTEQ);
            return NOTEQ;
            break;
        case 22:
            addTk(GREATER);
            return GREATER;
            break;
        case 23:
            addTk(GREATEREQ);
            return GREATEREQ;
            break;
        case 25:
            addTk(ASSIGN);
            return ASSIGN;
            break;
        case 26:
            addTk(EQUAL);
            return EQUAL;
            break;
        case 28:
            addTk(LESSEQ);
            return LESSEQ;
            break;
        case 29:
            addTk(LESS);
            return LESS;
            break;

            //delimitatori + operatori + comentarii stari intermediare
        case 12:
            switch (ch) {
            case '/':
                pCrtCh++;
                state = 32;
                break;
            case '*':
                pCrtCh++;
                state = 33;
                break;
            default:
                state = 54;
            }
            break;
        case 14:
            if (ch == '|') {
                pCrtCh++;
                state = 15;
            }
            else {
                tkerr(addTk(OR), "Invalid character %c after |, expected another |", ch);
            }
            break;
        case 16:
            if (ch == '&') {
                pCrtCh++;
                state = 17;
            }
            else {
                tkerr(addTk(OR), "Invalid character %c after &, expected another &", ch);
            }
            break;
        case 18:
            if (ch == '=') {
                pCrtCh++;
                state = 20;
            }
            else state = 19;
            break;
        case 21:
            if (ch == '=') {
                pCrtCh++;
                state = 23;
            }
            else state = 22;
            break;
        case 24:
            if (ch == '=') {
                pCrtCh++;
                state = 26;
            }
            else state = 25;
            break;
        case 27:
            if (ch == '=') {
                pCrtCh++;
                state = 28;
            }
            else state = 29;
            break;
        case 32:
            if (ch == '\n'){
                line++;
                state = 0;
            }
            else if (ch == '\r' || ch == '\0') {
                state = 0; //nu am facut pCrtCh++ pt ca in starea 0 trebuie tratate valorile, daca e linie nou (\n,\r) sau s-a terminat sirul de input (\0)
            }
            else {
                pCrtCh++;
                state = 32;
            }
            break;
        case 33:
            if (ch == '*') {
                pCrtCh++;
                state = 34;
            }else {
                if (ch == '\n') line++;
                pCrtCh++;
                state = 33;
            }
            break;
        case 34:
            if (ch == '*') {
                pCrtCh++;
                state = 34;
            }else if (ch == '/') {
                pCrtCh++;
                state = 0;
            }else {
                if (ch == '\n') line++;
                pCrtCh++;
                state = 33;
            }
            break;

        //constante char
        case 48:
            pCrtCh++;
            if (ch == '\\') state = 50;
            else state = 55;
            break;
        case 50:
            if (strchr("abfnrtv\'?\"\\0", ch) != NULL) {
                pCrtCh++;
                state = 55;
            }
            else if (ch == '\0') tkerr(addTk(CT_CHAR), "missing file end");
            else tkerr(addTk(CT_CHAR),"caracter special nesuportat/inexistent in constanata de tip char");
            break;
        case 55:
            if (ch == '\'') {
                pCrtCh++;
                state = 49;
            }
            else tkerr(addTk(CT_CHAR), "Eroare la definirea constantei de tip char, constanta trebue sa fie de forma \'c\', cu c fiind un caracter sau un caracter special de forma \\...");
        case 49://tocken de tipul CT_CHAR
            tk = addTk(CT_CHAR);
            if (pStartCh[1] == '\\') {
                //special char : a b f n r t v ' ? " \ 0
                tk->i = (char)makeSpecial(pStartCh[2]);
            }
            else {
                tk->i = (int)pStartCh[0]; //daca nu incepe cu backslash
            }
            return CT_CHAR;
            break;

        //constante string
        case 51:
            pCrtCh++;
            if (ch == '\\') state = 53;
            else if (ch == '\"') state = 52;
            else if (ch == '\0') tkerr(addTk(CT_STRING), "Sir de caractere neterminat pana la finalul fisierului");
            //pt orice alt caracter nu este nevoie sa schimbam starea
            break;
        case 53:
            if (strchr("abfnrtv\'?\"\\0", ch) != NULL) {
                pCrtCh++;
                state = 51;
            }
            else tkerr(addTk(CT_STRING), "caracter special nesuportat/inexistent in constanata de tip string");
            break;
        case 52:
            tk = addTk(CT_STRING);
            pStartCh++;//sar peste primul set de ghilimele
            tk->text = makeString(pStartCh, pCrtCh-1, 1); //-1 ca sa nu ia in considerare " de la sfarsit
            return CT_CHAR;
            break;
        //constante int
        case 41:
            if (isdigit(ch)) {
                pCrtCh++;
            }
            else if (ch == '.') {
                pCrtCh++;
                state = 42;
            }
            else if (ch == 'e' || ch == 'E') {
                pCrtCh++;
                state = 44;
            }
            else {
                state = 38;
            }
            break;
        case 35:
            if (ch == '8' || ch == '9') {
                pCrtCh++;
                state = 40;
            }
            else if (isdigit(ch)) { //orice alt numar, mai putin 8 si 9 care am verificat deja
                pCrtCh++;
                state = 39;
            }
            else {
                switch (ch) {
                case 'x':
                    pCrtCh++;
                    state = 36;
                    break;
                case '.':
                    pCrtCh++;
                    state = 42;
                    break;
                case 'e': case 'E':
                    pCrtCh++;
                    state = 44;
                    break;
                default:
                    state = 38;
                }
            }
            break;
        case 39:
            if (ch >= '0' && ch <= '7') { //este o cifra intre 0 si 7, inclusiv
                pCrtCh++;
            }
            else if (ch == '8' || ch == '9') { //am updatat automatul, am observat abia acum ca am uitat cand l-am transcris pe curat 2-3 cazuri care pornesc din nodul 39;
                pCrtCh++;
                state = 40;
            }
            else if (ch == '.') {
                pCrtCh++;
                state = 42;
            }
            else if (ch == 'e' || ch == 'E') {
                pCrtCh++;
                state = 44;
            }
            else state = 38;
            break;
        case 36:
            if (isalnum(ch)) {
                pCrtCh++;
                state = 37;
            }
            else tkerr(addTk(CT_INT), "eroare la declararea constantei int in forma hexazecimala");
            break;
        case 37:
            if (isalnum(ch)) {
                pCrtCh++;
            }
            else state = 38;
            break;
        case 38:
            tk = addTk(CT_INT);
            tk->i = makeInt(pStartCh);
            return CT_INT;
            break;
        //constante real
        case 42:
            if (isdigit(ch)) {
                pCrtCh++;
                state = 43;
            }
            else tkerr(addTk(CT_REAL), "Eroare la definirea constantei de tip real %s", makeString(pStartCh, pCrtCh, 0));
            break;
        case 43:
            if (isdigit(ch)) {
                pCrtCh++;
            }
            else if (ch == 'E' || ch == 'e') {
                pCrtCh++;
                state = 44;
            }
            else state = 47;
            break;
        case 44:
            if (ch == '+' || ch == '-')
                pCrtCh++;
            state = 45;
            break;
        case 45:
            if (isdigit(ch)) {
                pCrtCh++;
                state = 46;
            }
            else tkerr(addTk(CT_REAL), "Eroare la definirea constantei de tip real %s", makeString(pStartCh, pCrtCh, 0));
            break;
        case 46:
            if (isdigit(ch)) {
                pCrtCh++;
            }
            else state = 47;
            break;
        case 47:
            tmp = makeString(pStartCh, pCrtCh, 0);
            tk = addTk(CT_REAL);
            tk->r = atof(tmp);
            free(tmp);
            return CT_REAL;
            break;

        //id si cuvinte cheie
        case 30:
            if (isalnum(ch) || ch == '_') {
                pCrtCh++;
            }
            else state = 31;
            break;
        case 31:
            nCh = pCrtCh - pStartCh;
            //poate nu e foarte clar cu macroul, da ma lasa sa modific lucrurile mult mai usor si mai sigur la nevoie
#ifdef TEST_KEY
#error
#endif //TEST_KEY 

#define TEST_KEY(dimct, key, tk_key) if(nCh == dimct && memcmp(pStartCh, key, dimct) == 0) tk = addTk(tk_key); else          
            TEST_KEY(5, "break", BREAK)
            TEST_KEY(4, "char", CHAR)
            TEST_KEY(6, "double", DOUBLE)
            TEST_KEY(4, "else", ELSE)
            TEST_KEY(3, "for", FOR)
            TEST_KEY(2, "if", IF)
            TEST_KEY(3, "int", INT)
            TEST_KEY(6, "return", RETURN)
            TEST_KEY(6, "struct", STRUCT)
            TEST_KEY(4, "void", VOID)
            TEST_KEY(5, "while", WHILE)
            {
                tk = addTk(ID);
                tk->text = makeString(pStartCh, pCrtCh, 0);
            }
#undef TEST_KEY
            return tk->code;
        } //switch(state)
    } //while(1)
}//getNextTocken()    

    
    
