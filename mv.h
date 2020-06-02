#ifndef __mv_h__included__
#define __mv_h__included__

enum EOPCODE{
	O_ADD_C=0, O_ADD_D = 1, O_ADD_I = 2,												// \/
	O_AND_A=3, O_AND_C=4, O_AND_D=5, O_AND_I=6,											// \/
	O_CALL=7, O_CALLEXT=8,														// \/
	O_CAST_C_D=9, O_CAST_C_I=10, O_CAST_D_C=11, O_CAST_D_I=12, O_CAST_I_C=13, O_CAST_I_D=14, // \/
	O_DIV_C=15, O_DIV_D=16, O_DIV_I=17,												// \/
	O_DROP=18,																	// \/
	O_ENTER=19,																// \/
	O_EQ_A=20, O_EQ_C=21, O_EQ_D=22, O_EQ_I=23,								// \/
	O_GREATER_C=24, O_GREATER_D=25, O_GREATER_I=26,							// \/
	O_GREATEREQ_C=27, O_GREATEREQ_D=28, O_GREATEREQ_I=29,					// \/
	O_HALT=30,																// \/
	O_INSERT=31,															// \/
	O_JF_A, O_JF_C, O_JF_D, O_JF_I,											// \/										
	O_JMP,																	// \/
	O_JT_A, O_JT_C, O_JT_D, O_JT_I,											// \/
	O_LESS_C, O_LESS_D, O_LESS_I,											// \/
	O_LESSEQ_C, O_LESSEQ_D, O_LESSEQ_I,										// \/
	O_LOAD,																	// \/
	O_MUL_C, O_MUL_D, O_MUL_I,												// \/
	O_NEG_C, O_NEG_D, O_NEG_I,												// \/
	O_NOP,																	// \/
	O_NOT_A, O_NOT_C, O_NOT_D, O_NOT_I,										// \/
	O_NOTEQ_A, O_NOTEQ_C, O_NOTEQ_D, O_NOTEQ_I,								// \/
	O_OFFSET,																// \/
	O_OR_A, O_OR_C, O_OR_D, O_OR_I,											// \/
	O_PUSHFPADDR,															// \/
	O_PUSHCT_A, O_PUSHCT_C, O_PUSHCT_D, O_PUSHCT_I,							// \/
	O_RET,																	// \/
	O_STORE,																// \/
	O_SUB_C, O_SUB_D, O_SUB_I												// \/
}; // all opcodes; each one starts with O_
typedef struct _Instr {
	enum EOPCODE opcode; // O_*
	union {
		long int i; // int, char
		double d;
		void* addr;
	}args[2];
	struct _Instr* last, * next; // links to last, next instructions
}Instr;
extern Instr* instructions, * lastInstruction; // double linked list

extern void mvTest(void);
extern void run(Instr* IP);

void pushd(double d);
double popd();
void pusha(void* a);
void* popa();
void pushi(int d);
int popi();
void pushc(char c);
char popc();

//instruction manipulation code
extern Instr* addInstrI(int opcode, long int val);
extern Instr* addInstrII(int opcode, long int val1, long int val2);
extern Instr* addInstrA(int opcode, void* addr);
extern void deleteInstructionsAfter(Instr* start);
extern Instr* addInstrAfter(Instr* after, int opcode);
extern Instr* addInstr(int opcode);
extern void insertInstrAfter(Instr* after, Instr* i);
extern Instr* createInstr(int opcode);
extern void* allocGlobal(int size);

#endif
