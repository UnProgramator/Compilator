#ifndef __atip_h__included__
#define __atip_h__included__

#include "adom.h"

#define ATIP_START
#define ATIP_END

typedef union {
	long int i; // int, char
	double d; // double
	const char* str; // char[]
}CtVal;

typedef struct {
	Type type; // type of the result
	int isLVal; // if it is a LVal
	int isCtVal; // if it is a constant value (int, real, char, char[])
	CtVal ctVal; // the constat value
}RetVal;

extern Type createType(int typeBase, int nElements);
extern void cast(Type* dst, Type* src);
extern Type getArithType(Type* s1, Type* s2);


#endif
