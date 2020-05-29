#include "atip.h"

extern Token* crtToken;
#define crtTk crtToken

Type createType(int typeBase, int nElements)
{
	Type t;
	t.typeBase = typeBase;
	t.nElements = nElements;
	return t;
}

void cast(Type* dst, Type* src)
{
	if (src->nElements > -1) {
		if (dst->nElements > -1) {
			if (src->typeBase != dst->typeBase)
				tkerr(crtTk, "an array cannot be converted to an array of another type");
		}
		else {
			tkerr(crtTk, "an array cannot be converted to a non-array");
		}
	}
	else {
		if (dst->nElements > -1) {
			tkerr(crtTk, "a non-array cannot be converted to an array");
		}
	}
	switch (src->typeBase) {
	case TB_CHAR:
	case TB_INT:
	case TB_DOUBLE:
			switch (dst->typeBase) {
			case TB_CHAR:
			case TB_INT:
			case TB_DOUBLE:
				return;
			}
	case TB_STRUCT:
		if (dst->typeBase == TB_STRUCT) {
			if (src->struct_elems != dst->struct_elems)
				tkerr(crtTk, "a structure cannot be converted to another one");
			return;
		}
	}
	tkerr(crtTk, "incompatible types");
}



Type getArithType(Type* s1, Type* s2) {
	//if (s1->typeBase == s2->typeBase) return *s1; // nu functioneaza pt ca trebuie sa verific sa nu fie o functie sau o structura
	if (s1->typeBase == TB_VOID || s2->typeBase == TB_VOID) //verific daca un op e fo functie care returneaza void
		tkerr(crtToken, "o functie care returneaza void nu poate fi folosita in operatii aritmetice");

	if (s1->typeBase == TB_STRUCT || s2->typeBase == TB_STRUCT) //verific sa nu fie o variabila de tipul struct
		tkerr(crtToken, "oo variabila de tip structura nu poate fi folosita in operatii aritmetice");

	if(s1->nElements > -1 || s2->nElements > -1) //verific sa nu fie un array
		tkerr(crtToken, "un vector nu poate fi folosita in operatii aritmetice");
	
	switch (s1->typeBase) {
	case TB_CHAR:
		return *s2; //indiferent daca s2 defineste un int sau un double, tipul char va fi convertit la acesta, fiind cel mai putin prioritar; 
					//daca s2 e tot char nu va avea loc practic nici o conversie
	case TB_INT:
		if (s2->typeBase == TB_CHAR)
			return *s1;
		else
			return *s2; // daca s2 nu e tipul char, at e double sau int, int va fi convertit la double sau se ret int

	case TB_DOUBLE:
		return *s1; // indiferent daca s2 e char sau int sau double, acesta va fi convrtit la double
		break;
	}
}

