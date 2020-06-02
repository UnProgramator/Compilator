#ifndef _GC_H__INCLUDED_
#define _GC_H__INCLUDED_

#include "atip.h"
#include "adom.h"
#include "mv.h"

extern int sizeArgs, offset;

extern Instr* labelMain;
extern Instr* crtLoopEnd;

extern int typeBaseSize(Type* type);
extern int typeFullSize(Type* type);
extern int typeArgSize(Type* type);
extern Instr* getRVal(RetVal* rv);
extern void addCastInstr(Instr* after, Type* actualType, Type* neededType);
extern Instr* createCondJmp(RetVal* rv);

extern void printInstr(Instr* start);

#endif