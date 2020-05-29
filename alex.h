#ifndef _ALEX_H__INCLUDED_
#define _ALEX_H__INCLUDED_

#include "token.h"

extern Token* AnalizorLexical(char* InputFilePointer);
#define ALEX(st) AnalizorLexical(st)

#endif