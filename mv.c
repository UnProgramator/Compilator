#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mv.h"
#include "adom.h"
#include "atip.h"

#define STACK_SIZE (32*1024)
char stack[STACK_SIZE];
char* SP; // Stack Pointer
char* stackAfter; // first byte after stack; used for stack limit tests

#define GLOBAL_SIZE (32*1024)
char globals[GLOBAL_SIZE];
int nGlobals;
void* allocGlobal(int size);

Instr * instructions = NULL, * lastInstruction=NULL;

Instr * createInstr(int opcode)
{
	Instr* i;
	SAFEALLOC(i, Instr)
	i->opcode = opcode;
	return i;
}
void insertInstrAfter(Instr* after, Instr* i)
{
	i->next = after->next;
	i->last = after;
	after->next = i;
	if (i->next == NULL)lastInstruction = i;
}
Instr* addInstr(int opcode)
{
	Instr* i = createInstr(opcode);
	i->next = NULL;
	i->last = lastInstruction;
	if (lastInstruction) {
		lastInstruction->next = i;
	}
	else {
		instructions = i;
	}
	lastInstruction = i;
	return i;
}
Instr* addInstrAfter(Instr* after, int opcode)
{
	Instr* i = createInstr(opcode);
	insertInstrAfter(after, i);
	return i;
}

void pushd(double d)
{
	if (SP + sizeof(double) > stackAfter)err("out of stack");
	*(double*)SP = d;
	SP += sizeof(double);
}
double popd()
{
	SP -= sizeof(double);
	if (SP < stack)err("not enough stack bytes for popd");
	return *(double*)SP;
}
void pusha(void* a)
{
	if (SP + sizeof(void*) > stackAfter)err("out of stack");
	*(void**)SP = a;
	SP += sizeof(void*);
}
void* popa()
{
	SP -= sizeof(void*);
	if (SP < stack)err("not enough stack bytes for popa");
	return *(void**)SP;
}
void pushi(int d)
{
	if (SP + sizeof(int) > stackAfter)err("out of stack");
	*(int*)SP = d;
	SP += sizeof(int);
}
int popi()
{
	SP -= sizeof(int);
	if (SP < stack)err("not enough stack bytes for popd");
	return *(int*)SP;
}
void pushc(char d)
{
	if (SP + sizeof(char) > stackAfter)err("out of stack");
	*(char*)SP = d;
	SP += sizeof(char);
}
char popc()
{
	SP -= sizeof(char);
	if (SP < stack)err("not enough stack bytes for popd");
	return *(char*)SP;
}

Instr* addInstrA(int opcode, void* addr) { //– adauga o instructiune setandu-i si arg[0].addr
	Instr* retval = addInstr(opcode);
	retval->args[0].addr = addr;

	return retval;
}
Instr* addInstrI(int opcode, long int val) { //– adauga o instructiune setandu-i si arg[0].i
	Instr* retval = addInstr(opcode);
	retval->args[0].i = val;

	return retval;
}
Instr* addInstrII(int opcode, long int val1, long int val2) {//– adauga o instructiune setandu-i si arg[0].i, arg[1].i
	Instr* retval = addInstr(opcode);
	retval->args[0].i = val1;
	retval->args[1].i = val2;
	return retval;
}
void deleteInstructionsAfter(Instr* start) {//– sterge toate instructiunile de dupa instructiunea „start”
	Instr* tmp = start->next;
	start->next = NULL;
	lastInstruction = start;
	while (tmp) {
		start = tmp->next;
		free(tmp);
		tmp = start;
	}
}



void* allocGlobal(int size)
{
	void* p = globals + nGlobals;
	if (nGlobals + size > GLOBAL_SIZE)err("insufficient globals space");
	nGlobals += size;
	return p;
}

void mvTest()
{
	Instr* L1;
	long int* v = allocGlobal(sizeof(long int));
	addInstrA(O_PUSHCT_A, v);									// \/
	addInstrI(O_PUSHCT_I, 3);									// \/
	addInstrI(O_STORE, sizeof(long int));						// \/
	L1 = addInstrA(O_PUSHCT_A, v);								// \/
	addInstrI(O_LOAD, sizeof(long int));						// \/
	addInstrA(O_CALLEXT, findSymbol(&symbols, "put_i")->addr);  // \/
	addInstrA(O_PUSHCT_A, v);									// \/
	addInstrA(O_PUSHCT_A, v);									// \/
	addInstrI(O_LOAD, sizeof(long int));						// \/
	addInstrI(O_PUSHCT_I, 1);									// \/
	addInstr(O_SUB_I);											// \/
	addInstrI(O_STORE, sizeof(long int));						// \/
	addInstrA(O_PUSHCT_A, v);									// \/
	addInstrI(O_LOAD, sizeof(long int));						// \/
	addInstrA(O_JT_I, L1);										// \/
	addInstr(O_HALT);											// \/
}

void run(Instr* IP)
{
	int iVal1, iVal2;
	char cVal1, cVal2;
	double dVal1, dVal2;
	char* aVal1, * aVal2;
	char* FP = 0, * oldSP;
	SP = stack;
	stackAfter = stack + STACK_SIZE;
	while (1) {
		printf("%p/%d\t", IP, SP - stack);
		switch (IP->opcode) {
		//add
		case O_ADD_C:
			cVal1 = popc();
			cVal2 = popc();
			printf("ADD_C\t(%c+%c -> %c)\n", cVal2, cVal1, cVal2 + cVal1);
			pushc(cVal2 + cVal1);
			IP = IP->next;
			break;
		case O_ADD_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("ADD_D\t(%g+%g -> %g)\n", dVal2, dVal1, dVal2 + dVal1);
			pushd(dVal2 + dVal1);
			IP = IP->next;
			break;
		case O_ADD_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("ADD_I\t(%i+%i -> %i)\n", iVal2, iVal1, iVal2 + iVal1);
			pushi(iVal2 + iVal1);
			IP = IP->next;
			break;
		//and
		case O_AND_C:
			cVal1 = popc();
			cVal2 = popc();
			printf("ADD_C\t(%c&&%c -> %c)\n", cVal2, cVal1, cVal2 && cVal1);
			pushc(cVal2 && cVal1);
			IP = IP->next;
			break;
		case O_AND_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("ADD_D\t(%g&&%g -> %d)\n", dVal2, dVal1, dVal2 && dVal1);
			pushd(dVal2 && dVal1);
			IP = IP->next;
			break;
		case O_AND_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("ADD_I\t(%i&&%i -> %d)\n", iVal2, iVal1, iVal2 && iVal1);
			pushi(iVal2 && iVal1);
			IP = IP->next;
			break;
		//call
		case O_CALL:
			aVal1 = IP->args[0].addr;
			printf("CALL\t%p\n", aVal1);
			pusha(IP->next);
			IP = (Instr*)aVal1;
			break;
		case O_CALLEXT:
			printf("CALLEXT\t%p\n", IP->args[0].addr);
			(*(void(*)())IP->args[0].addr)();
			IP = IP->next;
			break;
		//cast
		case O_CAST_C_D:
			cVal1 = popc();
			dVal1 = (double)cVal1;
			printf("CAST_C_D\t(%c -> %g)\n", cVal1, dVal1);
			pushd(dVal1);
			IP = IP->next;
			break;
		case O_CAST_C_I:
			cVal1 = popc();
			iVal1 = (int)cVal1;
			printf("CAST_C_I\t(%c -> %d)\n", cVal1, iVal1);
			pushi(iVal1);
			IP = IP->next;
			break;
		case O_CAST_D_C:
			dVal1 = popd();
			cVal1 = (char)dVal1;
			printf("CAST_D_C\t(%g -> %c)\n", dVal1, cVal1);
			pushc(cVal1);
			IP = IP->next;
			break;
		case O_CAST_D_I:
			dVal1 = popd();
			iVal1 = (int)dVal1;
			printf("CAST_D_I\t(%g -> %d)\n", dVal1, iVal1);
			pushi(iVal1);
			IP = IP->next;
			break;
		case O_CAST_I_C:
			iVal1 = popi();
			cVal1 = (char)iVal1;
			printf("CAST_I_C\t(%d -> %c)\n", iVal1, cVal1);
			pushc(cVal1);
			IP = IP->next;
			break;
		case O_CAST_I_D:
			iVal1 = popi();
			dVal1 = (double)iVal1;
			printf("CAST_I_D\t(%d -> %g)\n", iVal1, dVal1);
			pushd(dVal1);
			IP = IP->next;
			break;
		//div
		case O_DIV_C:
			cVal1 = popc();
			cVal2 = popc();
			printf("DIV_C\t(%c/%c -> %c)\n", cVal2, cVal1, cVal2 / cVal1);
			pushc(cVal2 / cVal1);
			IP = IP->next;
			break;
		case O_DIV_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("DIV_D\t(%g/%g -> %g)\n", dVal2, dVal1, dVal2 / dVal1);
			pushd(dVal2 / dVal1);
			IP = IP->next;
			break;
		case O_DIV_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("DIV_I\t(%i/%i -> %i)\n", iVal2, iVal1, iVal2 / iVal1);
			pushi(iVal2 / iVal1);
			IP = IP->next;
			break;
		//drop
		case O_DROP:
			iVal1 = IP->args[0].i;
			printf("DROP\t%ld\n", iVal1);
			if (SP - iVal1 < stack)err("not enough stack bytes");
			SP -= iVal1;
			IP = IP->next;
			break;
		//enter
		case O_ENTER:
			iVal1 = IP->args[0].i;
			printf("ENTER\t%ld\n", iVal1);
			pusha(FP);
			FP = SP;
			SP += iVal1;
			IP = IP->next;
			break;
		//eq
		case O_EQ_A:
			aVal1 = popa();
			aVal2 = popa();
			printf("EQ_A\t(%p==%p -> %d)\n", aVal1, aVal2, aVal1 == aVal2);
			pushi(aVal1 == aVal2);
			IP = IP->next;
			break;
		case O_EQ_C:
			cVal1 = popc();
			cVal2 = popc();
			printf("EQ_C\t(%c==%c -> %d)\n", cVal2, cVal1, cVal2 == cVal1);
			pushi(cVal2 == cVal1);
			IP = IP->next;
			break;
		case O_EQ_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("EQ_I\t(%g==%g -> %d)\n", dVal2, dVal1, dVal2 == dVal1);
			pushi(dVal2 == dVal1);
			IP = IP->next;
			break;
		case O_EQ_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("EQ_D\t(%d==%d -> %d)\n", iVal2, iVal1, iVal2 == iVal1);
			pushi(iVal2 == iVal1);
			IP = IP->next;
			break;
		//greate
		case O_GREATER_C:
			cVal1 = popc();
			cVal2 = popc();
			printf("GR_C\t(%c>%c -> %d)\n", cVal2, cVal1, cVal2 > cVal1);
			pushi(cVal2 > cVal1);
			IP = IP->next;
			break;
		case O_GREATER_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("GR_D\t(%g>%g -> %d)\n", dVal2, dVal1, dVal2 > dVal1);
			pushi(dVal2 > dVal1);
			IP = IP->next;
			break;
		case O_GREATER_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("GR_I\t(%d>%d -> %d)\n", iVal2, iVal1, iVal2 > iVal1);
			pushi(iVal2 > iVal1);
			IP = IP->next;
			break;
		//greater-eq
		case O_GREATEREQ_C:
			cVal1 = popc();
			cVal2 = popc();
			printf("GEQ_C\t(%c>=%c -> %d)\n", cVal2, cVal1, cVal2 >= cVal1);
			pushi(cVal2 >= cVal1);
			IP = IP->next;
			break;
		case O_GREATEREQ_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("GEQ_D\t(%g>=%g -> %d)\n", dVal2, dVal1, dVal2 >= dVal1);
			pushi(dVal2 >= dVal1);
			IP = IP->next;
			break;
		case O_GREATEREQ_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("GEQ_I\t(%d>=%d -> %d)\n", iVal2, iVal1, iVal2 >= iVal1);
			pushi(iVal2 >= iVal1);
			IP = IP->next;
			break;
		//halt
		case O_HALT:
			printf("HALT\n");
			return;
		case O_INSERT:
			iVal1 = IP->args[0].i; // iDst
			iVal2 = IP->args[1].i; // nBytes
			printf("INSERT\t%ld,%ld\n", iVal1, iVal2);
			if (SP + iVal2 > stackAfter)err("out of stack");
			memmove(SP - iVal1 + iVal2, SP - iVal1, iVal1); //make room
			memmove(SP - iVal1, SP + iVal2, iVal2); //dup
			SP += iVal2;
			IP = IP->next;
			break;
		//jumps
		case O_JF_A:
			aVal1 = popa();
			printf("JF_A\t%p\t(%p)\n", IP->args[0].addr, aVal1);
			IP = !aVal1 ? IP->args[0].addr : IP->next;
			break;
		case O_JF_C:
			cVal1 = popc();
			printf("JF_C\t%p\t(%c)\n", IP->args[0].addr, cVal1);
			IP = !cVal1 ? IP->args[0].addr : IP->next;
			break;
		case O_JF_D:
			dVal1 = popd();
			printf("JF_D\t%p\t(%g)\n", IP->args[0].addr, dVal1);
			IP = !dVal1 ? IP->args[0].addr : IP->next;
			break;
		case O_JF_I:
			iVal1 = popi();
			printf("JF_I\t%p\t(%ld)\n", IP->args[0].addr, iVal1);
			IP = !iVal1 ? IP->args[0].addr : IP->next;
			break;
		case O_JMP:
			printf("JMP\t%p\n", IP->args[0].addr);
			IP = IP->args[0].addr;
			break;
		case O_JT_A:
			aVal1 = popa();
			printf("JT_A\t%p\t(%p)\n", IP->args[0].addr, aVal1);
			IP = aVal1 ? IP->args[0].addr : IP->next;
			break;
		case O_JT_C:
			cVal1 = popc();
			printf("JT_C\t%p\t(%c)\n", IP->args[0].addr, cVal1);
			IP = cVal1 ? IP->args[0].addr : IP->next;
			break;
		case O_JT_D:
			dVal1 = popd();
			printf("JT_D\t%p\t(%g)\n", IP->args[0].addr, dVal1);
			IP = dVal1 ? IP->args[0].addr : IP->next;
			break;
		case O_JT_I:
			iVal1 = popi();
			printf("JT_I\t%p\t(%ld)\n", IP->args[0].addr, iVal1);
			IP = iVal1 ? IP->args[0].addr : IP->next;
			break;
		//less
		case O_LESS_C:
			cVal1 = popc();
			cVal2 = popc();
			printf("LS_C\t(%c<%c -> %d)\n", cVal2, cVal1, cVal2 < cVal1);
			pushi(cVal2 < cVal1);
			IP = IP->next;
			break;
		case O_LESS_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("LS_D\t(%g<%g -> %d)\n", dVal2, dVal1, dVal2 < dVal1);
			pushi(dVal2 < dVal1);
			IP = IP->next;
			break;
		case O_LESS_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("LS_I\t(%d<%d -> %d)\n", iVal2, iVal1, iVal2 < iVal1);
			pushi(iVal2 < iVal1);
			IP = IP->next;
			break;
		//less-eq
		case O_LESSEQ_C:
			cVal1 = popc();
			cVal2 = popc();
			printf("LEQ_C\t(%c<=%c -> %d)\n", cVal2, cVal1, cVal2 <= cVal1);
			pushi(cVal2 <= cVal1);
			IP = IP->next;
			break;
		case O_LESSEQ_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("LEQ_D\t(%g<=%g -> %d)\n", dVal2, dVal1, dVal2 <= dVal1);
			pushi(dVal2 <= dVal1);
			IP = IP->next;
			break;
		case O_LESSEQ_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("LEQ_I\t(%d<=%d -> %d)\n", iVal2, iVal1, iVal2 <= iVal1);
			pushi(iVal2 <= iVal1);
			IP = IP->next;
			break;
		//load
		case O_LOAD:
			iVal1 = IP->args[0].i;
			aVal1 = popa();
			printf("LOAD\t%ld\t(%p)\n", iVal1, aVal1);
			if (SP + iVal1 > stackAfter)err("out of stack");
			memcpy(SP, aVal1, iVal1);
			SP += iVal1;
			IP = IP->next;
			break;
		//mul
		case O_MUL_C:
			cVal1 = popc();
			cVal2 = popc();
			printf("MUL_C\t(%c*%c -> %c)\n", cVal2, cVal1, cVal2 * cVal1);
			pushc(cVal2 * cVal1);
			IP = IP->next;
			break;
		case O_MUL_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("MUL_D\t(%g*%g -> %g)\n", dVal2, dVal1, dVal2 * dVal1);
			pushd(dVal2 * dVal1);
			IP = IP->next;
			break;
		case O_MUL_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("MUL_I\t(%i*%i -> %i)\n", iVal2, iVal1, iVal2 * iVal1);
			pushi(iVal2 * iVal1);
			IP = IP->next;
			break;
		//neg
		case O_NEG_C:
			cVal1 = popc();
			printf("NEG_C\t(-%i(-%c) -> %i(%c))\n", cVal1, cVal1, -cVal1, -cVal1);
			pushc(-cVal1);
			IP = IP->next;
			break;
		case O_NEG_D:
			dVal1 = popd();
			printf("NEG_D\t(-%g -> %g)\n", dVal1, -dVal1);
			pushi(-dVal1);
			IP = IP->next;
			break;
		case O_NEG_I:
			iVal1 = popi();
			printf("NEG_I\t(-%i -> %i)\n", iVal1, -iVal1);
			pushi(-iVal1);
			IP = IP->next;
			break;
		//nop
		case O_NOP:
			printf("NOP_I\n");
			IP = IP->next;
			break;
		//not
		case O_NOT_A:
			aVal1 = popa();
			printf("NOT_A\t(!%p -> %d)\n", aVal1, !aVal1);
			pushi(!aVal1);
			IP = IP->next;
			break;
		case O_NOT_C:
			cVal1 = popc();
			printf("NOT_C\t(!%i(!%c) -> %i(%c))\n", cVal1, cVal1, !cVal1, !cVal1);
			pushi(!cVal1);
			IP = IP->next;
			break;
		case O_NOT_D:
			dVal1 = popd();
			printf("NOT_D\t(!%g -> %d)\n", dVal1, !dVal1);
			pushi(!dVal1);
			IP = IP->next;
			break;
		case O_NOT_I:
			iVal1 = popi();
			printf("NOT_I\t(!%i -> %i)\n", iVal1, !iVal1);
			pushi(!iVal1);
			IP = IP->next;
			break;
		//noteq
		case O_NOTEQ_A:
			aVal1 = popa();
			aVal2 = popa();
			printf("EQ_A\t(%p!=%p -> %d)\n", aVal1, aVal2, aVal1 != aVal2);
			pushi(aVal1 != aVal2);
			IP = IP->next;
			break;
		case O_NOTEQ_C:
			cVal1 = popc();
			cVal2 = popc();
			printf("EQ_C\t(%c!=%c -> %d)\n", cVal2, cVal1, cVal2 != cVal1);
			pushi(cVal2 != cVal1);
			IP = IP->next;
			break;
		case O_NOTEQ_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("EQ_I\t(%g!=%g -> %d)\n", dVal2, dVal1, dVal2 != dVal1);
			pushi(dVal2 != dVal1);
			IP = IP->next;
			break;
		case O_NOTEQ_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("EQ_D\t(%d!=%d -> %d)\n", iVal2, iVal1, iVal2 != iVal1);
			pushi(iVal2 != iVal1);
			IP = IP->next;
			break;
		//offset
		case O_OFFSET:
			iVal1 = popi();
			aVal1 = popa();
			printf("OFFSET\t(%p+%ld -> %p)\n", aVal1, iVal1, aVal1 + iVal1);
			pusha(aVal1 + iVal1);
			IP = IP->next;
			break;
		//or
		case O_OR_A:
			aVal1 = popa();
			aVal2 = popa();
			printf("EQ_A\t(%p||%p -> %d)\n", aVal1, aVal2, aVal1 || aVal2);
			pushi(aVal1 || aVal2);
			IP = IP->next;
			break;
		case O_OR_C:
			cVal1 = popc();
			cVal2 = popc();
			printf("EQ_C\t(%c||%c -> %d)\n", cVal2, cVal1, cVal2 || cVal1);
			pushi(cVal2 || cVal1);
			IP = IP->next;
			break;
		case O_OR_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("EQ_I\t(%g||%g -> %d)\n", dVal2, dVal1, dVal2 || dVal1);
			pushi(dVal2 || dVal1);
			IP = IP->next;
			break;
		case O_OR_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("EQ_D\t(%d||%d -> %d)\n", iVal2, iVal1, iVal2 || iVal1);
			pushi(iVal2 || iVal1);
			IP = IP->next;
			break;
		//push
		case O_PUSHFPADDR:
			iVal1 = IP->args[0].i;
			printf("PUSHFPADDR\t%ld\t(%p)\n", iVal1, FP + iVal1);
			pusha(FP + iVal1);
			IP = IP->next;
			break;
		case O_PUSHCT_A:
			aVal1 = IP->args[0].addr;
			printf("PUSHCT_A\t%p\n", aVal1);
			pusha(aVal1);
			IP = IP->next;
			break;
		case O_PUSHCT_I:
			iVal1 = IP->args[0].i;
			printf("PUSHCT_I\t%d\n", iVal1);
			pushi(iVal1);
			IP = IP->next;
			break;
		case O_PUSHCT_C:
			cVal1 = IP->args[0].i;
			printf("PUSHCT_C\t%c\n", cVal1);
			pushc(cVal1);
			IP = IP->next;
			break;
		case O_PUSHCT_D:
			dVal1 = IP->args[0].d;
			printf("PUSHCT_D\t%g\n", dVal1);
			pushd(dVal1);
			IP = IP->next;
			break;
		case O_RET:
			iVal1 = IP->args[0].i; // sizeArgs
			iVal2 = IP->args[1].i; // sizeof(retType)
			printf("RET\t%ld,%ld\n", iVal1, iVal2);
			oldSP = SP;
			SP = FP;
			FP = popa();
			IP = popa();
			if (SP - iVal1 < stack)err("not enough stack bytes");
			SP -= iVal1;
			memmove(SP, oldSP - iVal2, iVal2);
			SP += iVal2;
			break;
		case O_STORE:
			iVal1 = IP->args[0].i;
			if (SP - (sizeof(void*) + iVal1) < stack)err("not enough stack bytes for SET");
			aVal1 = *(void**)(SP - ((sizeof(void*) + iVal1)));
			printf("STORE\t%ld\t(%p)\n", iVal1, aVal1);
			memcpy(aVal1, SP - iVal1, iVal1);
			SP -= sizeof(void*) + iVal1;
			IP = IP->next;
			break;
		//sub
		case O_SUB_C:
			cVal1 = popc();
			cVal2 = popc();
			printf("SUB_C\t(%c-%c -> %c)\n", cVal2, cVal1, cVal2 - cVal1);
			pushc(cVal2 - cVal1);
			IP = IP->next;
			break;
		case O_SUB_D:
			dVal1 = popd();
			dVal2 = popd();
			printf("SUB_D\t(%g-%g -> %g)\n", dVal2, dVal1, dVal2 - dVal1);
			pushd(dVal2 - dVal1);
			IP = IP->next;
			break;
		case O_SUB_I:
			iVal1 = popi();
			iVal2 = popi();
			printf("SUB_I\t(%i-%i -> %i)\n", iVal2, iVal1, iVal2 - iVal1);
			pushi(iVal2 - iVal1);
			IP = IP->next;
			break;
		default:
			err("invalid opcode: %d", IP->opcode);
		}
	}
}

