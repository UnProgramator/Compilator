#include "gc.h"
#include "atip.h"
#include "adom.h"
#include "mv.h"

#include <stdlib.h>

extern Token * crtToken;

int sizeArgs, offset;
Instr* labelMain;
Instr* crtLoopEnd;

int typeBaseSize(Type* type)
{
	int size = 0;
	Symbol** is;
		switch (type->typeBase) {
		case TB_INT:size = sizeof(long int); break;
		case TB_DOUBLE:size = sizeof(double); break;
		case TB_CHAR:size = sizeof(char); break;
		case TB_STRUCT:
			for (is = type->struct_elems->members.begin; is != type->struct_elems->members.end; is++) {
				size += typeFullSize(&(*is)->type);
			}
			break;
		case TB_VOID:size = 0; break;
		default:err("invalid typeBase: %d", type->typeBase);
		}
	return size;
}

int typeFullSize(Type* type)
{
	return typeBaseSize(type) * (type->nElements > 0 ? type->nElements : 1);
}

int typeArgSize(Type* type)
{
	if (type->nElements >= 0)return sizeof(void*);
	return typeBaseSize(type);
}

Instr* getRVal(RetVal* rv)
{
	if (rv->isLVal) {
		switch (rv->type.typeBase) {
		case TB_INT:
		case TB_DOUBLE:
		case TB_CHAR:
		case TB_STRUCT:
			addInstrI(O_LOAD, typeArgSize(&rv->type));
			break;
		default:tkerr(crtToken, "unhandled type: %d", rv->type.typeBase);
		}
	}
	return lastInstruction;
}

void addCastInstr(Instr* after, Type* actualType, Type* neededType)
{
	if (actualType->nElements >= 0 || neededType->nElements >= 0)return;
	switch (actualType->typeBase) {
	case TB_CHAR:
		switch (neededType->typeBase) {
		case TB_CHAR:break;
		case TB_INT:addInstrAfter(after, O_CAST_C_I); break;
		case TB_DOUBLE:addInstrAfter(after, O_CAST_C_D); break;
		}
		break;
	case TB_INT:
		switch (neededType->typeBase) {
		case TB_CHAR:addInstrAfter(after, O_CAST_I_C); break;
		case TB_INT:break;
		case TB_DOUBLE:addInstrAfter(after, O_CAST_I_D); break;
		}
		break;
	case TB_DOUBLE:
		switch (neededType->typeBase) {
		case TB_CHAR:addInstrAfter(after, O_CAST_D_C); break;
		case TB_INT:addInstrAfter(after, O_CAST_D_I); break;
		case TB_DOUBLE:break;
		}
		break;
	}
}

Instr* createCondJmp(RetVal* rv)
{
	if (rv->type.nElements >= 0) {  // arrays
		return addInstr(O_JF_A);
	}
	else {                              // non-arrays
		getRVal(rv);
		switch (rv->type.typeBase) {
		case TB_CHAR:return addInstr(O_JF_C);
		case TB_DOUBLE:return addInstr(O_JF_D);
		case TB_INT:return addInstr(O_JF_I);
		default:return NULL;
		}
	}
}


void printInstr(Instr* start) {
	while (start) {
		printf("%d -> %p\n", start->opcode, start->next);
		start = start->next;
	}
}