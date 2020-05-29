#include "externfunc.h"
#include "adom.h"
#include "atip.h"
#include "mv.h"
#include <stdio.h>
#include <stdlib.h>

Symbol* addExtFunc(const char* name, Type type, void*addr)
{
	Symbol* smb = addSymbol(&symbols, name, CLS_EXTFUNC);
	smb->type = type;
	smb->addr = addr;
	initSymbols(&smb->args);
	return smb;
}
Symbol* addFuncArg(Symbol* func, const char* name, Type type)
{
	Symbol* a = addSymbol(&func->args, name, CLS_VAR);
	a->type = type;
	return a;
}

void get_s();
void put_s();
void put_i();
void get_i();
void put_d();
void get_d();
void put_c();
void get_c();
void seconds();

void initExtFunc(void) {
	Symbol* crtSymbol = addExtFunc("put_s", createType(TB_VOID, -1), &put_s);
	addFuncArg(crtSymbol, "s", createType(TB_CHAR, 0));

	crtSymbol = addExtFunc("get_s", createType(TB_VOID, -1), &get_s);
	addFuncArg(crtSymbol, "s", createType(TB_CHAR, 0));

	crtSymbol = addExtFunc("put_i", createType(TB_VOID, -1), &put_i);
	addFuncArg(crtSymbol, "i", createType(TB_INT, -1));

	crtSymbol = addExtFunc("get_i", createType(TB_INT, -1), &get_i);

	crtSymbol = addExtFunc("put_d", createType(TB_VOID, -1), &put_d);
	addFuncArg(crtSymbol, "d", createType(TB_DOUBLE, -1));

	crtSymbol = addExtFunc("get_d", createType(TB_DOUBLE, -1), &get_d);

	crtSymbol = addExtFunc("put_c", createType(TB_VOID, -1), &put_c);
	addFuncArg(crtSymbol, "s", createType(TB_CHAR, -1));

	crtSymbol = addExtFunc("get_c", createType(TB_CHAR, -1), &get_c);

	crtSymbol = addExtFunc("seconds", createType(TB_DOUBLE, -1), &seconds);

}

void get_s()
{
	char* s = popa();
	fgets(s, 256, stdin);
}

void put_s()
{
	puts(popa());
}

void put_i()
{
	printf("#%ld\n", popi());
}

void get_i()
{
	int i;
	scanf("%d", &i);
	pushi(i);
}

void put_d()
{
	printf("#%lf\n", popd());
}

void get_d()
{
	int i;
	scanf("%d", &i);
	pushi(i);
}

void put_c()
{
	printf("#%ld\n", popc());
}

void get_c()
{
	pushc(getc(stdin));
	
}

void seconds()
{
	printf("#%ld\n", popi());
}


