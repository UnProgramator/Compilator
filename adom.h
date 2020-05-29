#ifndef __adom_h__included__
#define __adom_h__included__

#include "token.h"

struct _Symbol;
typedef struct _Symbol Symbol;
typedef struct {
	Symbol** begin; // the beginning of the symbols, or NULL
	Symbol** end; // the position after the last symbol
	Symbol** after; // the position after the allocated space
}Symbols;

enum EType{ TB_INT, TB_DOUBLE, TB_CHAR, TB_STRUCT, TB_VOID };
typedef struct {
	enum EType typeBase; // TB_*
	Symbol* struct_elems; // struct definition for TB_STRUCT
	int nElements; // >0 array of given size, 0=array without size, <0 non array
}Type;
enum CLS{ CLS_VAR, CLS_FUNC, CLS_EXTFUNC, CLS_STRUCT };
enum MEM{ MEM_GLOBAL, MEM_ARG, MEM_LOCAL };
typedef struct _Symbol {
	const char* name; // a reference to the name stored in a token
	enum CLS cls; // CLS_*
	enum MEM mem; // MEM_*
	Type type;
	int depth; // 0-global, 1-in function, 2... - nested blocks in function
	union {
		Symbols args; // used only of functions
		Symbols members; // used only for structs
	};
	union {
		void* addr; // vm: the memory address for global symbols
		int offset; // vm: the stack offset for local symbols
	};
}Symbol;

extern Symbols symbols;
extern int crtDepth;
extern Symbol* crtFunc;
extern Symbol* crtStruct;

void initSymbols(Symbols* symbols);
Symbol* addSymbol(Symbols* symbols, const char* name, enum CLS cls);
Symbol* findSymbol(Symbols* symbols, const char* name);
void deleteSymbolsAfter(Symbols* symbols, Symbol* lastOne);
void addVar(Token* tkName, Type* t);

#define ADOM_START(__name__)
#define ADOM_END

#endif