#include "adom.h"
#include "token.h"
#include "gc.h"
#include <stdlib.h>
#include <string.h>

Symbols symbols;
int crtDepth = 0;
Symbol* crtFunc;
Symbol* crtStruct;

void initSymbols(Symbols* symbols)
{
	symbols->begin = NULL;
	symbols->end = NULL;
	symbols->after = NULL;
}

Symbol* addSymbol(Symbols* symbols, const char* name, enum CLS cls)
{
	Symbol* struct_elems;
	if (symbols->end == symbols->after) { // create more room
		const int count = symbols->after - symbols->begin;
		int n = count * 2; // double the room
		if (n == 0){
			n = 10; // needed for the initial case
			symbols->begin = (Symbol**)malloc(n * sizeof(Symbol*));
		}	
		else
			symbols->begin = (Symbol**)realloc(symbols->begin, n * sizeof(Symbol*));
		if (symbols->begin == NULL) err("not enough memory");
		symbols->end = symbols->begin + count;
		symbols->after = symbols->begin + n;
	}
	SAFEALLOC(struct_elems, Symbol);
	*symbols->end++ = struct_elems;
	struct_elems->name = name;
	struct_elems->cls = cls;
	struct_elems->depth = crtDepth;
	return struct_elems;
}

Symbol* findSymbol(Symbols* symbols, const char* name)
{
	Symbol** tmp;

	if (symbols->begin == symbols->end)
		return NULL; // "NULL check" => nu avem nici un symbol in tabela; fie is egale fie is ambele pe null, caz in care sunt evident egale

	for (tmp = symbols->end - 1; tmp != symbols->begin; tmp--) // verific toate elementele mai putin primul 
		if (!strcmp((*tmp)->name, name))
			return *tmp;

	return (strcmp((*tmp)->name, name) ? NULL : *tmp); //verific primul element
}

void deleteSymbolsAfter(Symbols* symbols, Symbol* lastOne)
{
	Symbol** tmp = symbols->begin;
	while (*tmp != lastOne) // daca begin == NULL == after, iasa
		tmp++;
	symbols->end = tmp+1; // aici tmp are valoare * spre ultimul element, deci end e urmatorul dupa ultimul
}

void addVar(Token* tkName, Type* t)
{
	Symbol* struct_elems;
	if (crtStruct) {
		if (findSymbol(&crtStruct->members, tkName->text))
			tkerr(tkName, "symbol redefinition: %s", tkName->text);
		struct_elems = addSymbol(&crtStruct->members, tkName->text, CLS_VAR);
	}
	else if (crtFunc) {
		struct_elems = findSymbol(&symbols, tkName->text);
		if (struct_elems && struct_elems->depth == crtDepth)
			tkerr(tkName, "symbol redefinition: %s", tkName->text);
		if (struct_elems && struct_elems->cls == CLS_FUNC)
			tkerr(tkName, "simbolul deja exista ca o functie");
		struct_elems = addSymbol(&symbols, tkName->text, CLS_VAR);
		struct_elems->mem = MEM_LOCAL;
	}
	else {
		if (findSymbol(&symbols, tkName->text))
			tkerr(tkName, "symbol redefinition: %s", tkName->text);
		struct_elems = addSymbol(&symbols, tkName->text, CLS_VAR);
		struct_elems->mem = MEM_GLOBAL;
	}
	struct_elems->type = *t;

	//GC - start

	if (crtStruct || crtFunc) {
		struct_elems->offset = offset;
	}
	else {
		struct_elems->addr = allocGlobal(typeFullSize(&struct_elems->type));
	}
	offset += typeFullSize(&struct_elems->type);

	//GC - end
}