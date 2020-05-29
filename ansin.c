#include <stdio.h>

#include "token.h"
#include "ansin.h"
#include "adom.h"
#include "atip.h"
#include "mv.h"
#include "gc.h"

Token* crtToken;
static Token* consumedTk;

int unit();
int declStruct();
int declVar();
int typeBase(Type*);
int arrayDecl(Type*);
int typeName(Type* t);
int declFunc();
int funcArg();
int stm();
int stmCompound();
int expr(RetVal* rv);
int exprAssign(RetVal* rv);
int exprOr(RetVal* rv);
int exprAnd(RetVal* rv);
int exprEq(RetVal* rv);
int exprRel(RetVal* rv);
int exprAdd(RetVal* rv);
int exprMul(RetVal* rv);
int exprCast(RetVal* rv);
int exprUnary(RetVal* rv);
int exprPostfix(RetVal* rv);
int exprPrimary(RetVal* rv);

int AnalizorSintactic(Token* fstTok)
{
	crtToken = fstTok;
	return unit();
}

int consume(TkCode code) {
	if (crtToken->code == code) {
		consumedTk = crtToken;
		crtToken = crtToken->next;
		return 1;
	}
	else
		return 0;
}

//unit: ( declStruct | declFunc | declVar )* END ;
int unit() {
	Token* start = crtToken;
	Instr* labelMain = addInstr(O_CALL);
	addInstr(O_HALT);
	while (1) {
		if (declStruct()) {
			continue;
		}
		else if (declFunc()) {
			continue;
		}
		else if (declVar()) {
			continue;
		}
		else break;
	}
	if (consume(END)) {
		return 1;
	}
	labelMain->args[0].addr = findSymbol(&symbols, "main")->addr;
	crtToken = start;
	return 0;
}

//declStruct: STRUCT ID LACC declVar* RACC SEMICOLON ;
int declStruct() {
	Token* start = crtToken;
	Token* tkName;
	if (consume(STRUCT)) {
		tkName = crtToken;
		if (consume(ID)) {
			if (consume(LACC)) {
				//GC -start
				offset = 0;
				//GC -end
				ADOM_START(struct_name)
					if (findSymbol(&symbols, tkName->text))
						tkerr(crtToken, "symbol redefinition: %s", tkName->text);
					crtStruct = addSymbol(&symbols, tkName->text, CLS_STRUCT);
					initSymbols(&crtStruct->members);
				ADOM_END

				while (1) {
					if (declVar()) continue;
					else break;
				}
				if (consume(RACC)) {
					if (consume(SEMICOLON)) {
						ADOM_START(struct_name)
							crtStruct = NULL;
						ADOM_END
						return 1;
					}
					else tkerr(crtToken, "Lipseste punct si virgula ; dupa incheierea declaratiei structurii");
				}
				else tkerr(crtToken, "Lipseste acolada inchisa");
			}
		}
		else tkerr(crtToken, "numele structurii este invalid sau lipseste");
	}

	crtToken = start;
	return 0;
}

//declVar:  typeBase ID arrayDecl? ( COMMA ID arrayDecl? )* SEMICOLON ;
int declVar() {
	Token* start = crtToken;
	Token* tkName;
	Type t;
	if (typeBase(&t)) {
		if (consume(ID)) {
			tkName = consumedTk;
			if (!arrayDecl(&t)) {
				t.nElements = -1;
			}
			ADOM_START(declVar)
				addVar(tkName, &t);
			ADOM_END
			while (1) {
				if (consume(COMMA)) {
					if (consume(ID)) {
						tkName = consumedTk;
						if (!arrayDecl(&t)) {
							t.nElements = -1;
						}
						ADOM_START(declVar)
							addVar(tkName, &t);
						ADOM_END
					}
					else tkerr(crtToken, "Eroare de sintaxa, lipseste declaratie de variabila dupa virgula sau numele nu e valid");
				}
				else break;
			}
			if (consume(SEMICOLON)) return 1;
		}
		else {
			//nu pot avea altceva decat un id; daca era typeBase * putea fi functie, dar functia nu poate fi decat globala, si se verifica daca nu e fct inainte sa se verifice daca e variabila
			tkerr(crtToken, "Eroare la declarare, lipseste identificatorul sau are nume incorect");
		}
	}

	crtToken = start;
	return 0;
}

//typeBase: INT | DOUBLE | CHAR | STRUCT ID ;
int typeBase(Type* t) {
	Token* start = crtToken;
	if (consume(INT)) {
		ADOM_START(typeBase)
			t->typeBase = TB_INT;
		ADOM_END
		return 1;
	}
	else if (consume(DOUBLE)) {
		ADOM_START(typeBase)
			t->typeBase = TB_DOUBLE;
		ADOM_END
		return 1;
	}
	else if(consume(CHAR)) {
		ADOM_START(typeBase)
			t->typeBase = TB_CHAR;
		ADOM_END
		return 1;
	}
	else if(consume(STRUCT)) {
		if (consume(ID)) {
			ADOM_START(typeBase)
				Symbol* struct_elems;
				struct_elems = findSymbol(&symbols, consumedTk->text);
				if (struct_elems == NULL) tkerr(consumedTk, "undefined symbol: %s", consumedTk->text);
				else if (struct_elems->cls != CLS_STRUCT) tkerr(consumedTk, "%s is not a struct", consumedTk->text);
				t->typeBase = TB_STRUCT;
				t->struct_elems = struct_elems;
			ADOM_END
			return 1;
		}
		else tkerr(crtToken, "declaratie de tip structura gresita");
	}

	crtToken = start;
	return 0;
}

//arrayDecl: LBRACKET expr? RBRACKET ;
int arrayDecl(Type*ret) {
	Token* start = crtToken;
	Instr* instrBeforeExpr;
	if (consume(LBRACKET)) {
		RetVal rv;
		if (expr(&rv)) {
			instrBeforeExpr = lastInstruction; //GC
			ATIP_START
				if (!rv.isCtVal)tkerr(crtToken, "the array size is not a constant");
				if (rv.type.typeBase != TB_INT)tkerr(crtToken, "the array size is not an integer");
				ret->nElements = rv.ctVal.i;
				deleteInstructionsAfter(instrBeforeExpr); //GC
		}
		else
				ret->nElements = 0;/*array without given size*/
			ATIP_END
		if (consume(RBRACKET)) {
			return 1;
		}
		else tkerr(crtToken, "paranteze patrate deschisa dar nu si inchisa");
	}

	crtToken = start;
	return 0;
}

//typeName: typeBase arrayDecl? ;
int typeName(Type* t) {
	Token* start = crtToken;

	if (typeBase(NULL)) {
		if (!arrayDecl(t))
			t->nElements = -1;
		return 1;
	}

	crtToken = start;
	return 0;
}

/*declFunc: ( typeBase MUL? | VOID ) ID 
                        LPAR ( funcArg ( COMMA funcArg )* )? RPAR 
                        stmCompound ;	*/
int declFunc() {
	Token* start = crtToken;
	Token* tkName;
	int isFun = 0;
	Type t;


	if (typeBase(&t)) {
		if (consume(MUL)) {
			isFun = 1;
			t.nElements = 0;
		}
		else
			t.nElements = -1;
	}
	else if (consume(VOID)) {
		isFun = 1;
		t.typeBase = TB_VOID;
		t.nElements = -1;
	}
	else return 0;
	if (consume(ID)) { //daca nu am id si am { at e declaratie de struct
		tkName = consumedTk;
		if (consume(LPAR)) {

			ADOM_START(verify_func)
				if (findSymbol(&symbols, tkName->text))
					tkerr(tkName, "symbol redefinition: %s", tkName->text);
				crtFunc = addSymbol(&symbols, tkName->text, CLS_FUNC);
				initSymbols(&crtFunc->args);
				crtFunc->type = t;
				crtDepth++;
			ADOM_END

			if (funcArg()) {
				while (1) {
					if (consume(COMMA)) {
						if (funcArg) {
							continue;
						}
						else tkerr(crtToken, "declaratie de parametru invalida sau virgula in plus");
					}
					else break;
				}
			}

			if (consume(RPAR)) {
				ADOM_START(adjust_depth)
				crtDepth--;
				ADOM_END
				if (stmCompound()) { // 1- trebuie neaparat acolada deschisa - in caz ca nu e da eroare
					ADOM_START(elim_local_var)
						deleteSymbolsAfter(&symbols, crtFunc);
						crtFunc = NULL;
					ADOM_END
					return 1;
				}
				else tkerr(crtToken, "Corpul functiei nu are o forma valida; lipseste una dintre acolade/ nu sunt bine imperecheate");
			}
			else tkerr(crtToken, "forma parametriilor gresita sau paranteze neechilibrate");
		}
	}

	crtToken = start;
	return 0;
}

//funcArg: typeBase ID arrayDecl? ;
int funcArg() {
	Token* start = crtToken;
	Token* tkName;
	Type t;
	if (typeBase(&t)) {
		if (consume(ID)) {
			tkName = consumedTk;
			if (!arrayDecl(&t))
				t.nElements = -1;
			ADOM_START(funcArg)
				Symbol* struct_elems = addSymbol(&symbols, tkName->text, CLS_VAR);
				struct_elems->mem = MEM_ARG;
				struct_elems->type = t;
				struct_elems = addSymbol(&crtFunc->args, tkName->text, CLS_VAR);
				struct_elems->mem = MEM_ARG;
				struct_elems->type = t;
			ADOM_END
			return 1;
		}
		else tkerr(crtToken, "ID parametru functie invalid sau inexistent");
	}
	crtToken = start;
	return 0;
}

/*stm: stmCompound 
           | IF LPAR expr RPAR stm ( ELSE stm )?
           | WHILE LPAR expr RPAR stm
           | FOR LPAR expr? SEMICOLON expr? SEMICOLON expr? RPAR stm
           | BREAK SEMICOLON
           | RETURN expr? SEMICOLON
           | expr? SEMICOLON ;		*/
static int _ifStm() {
	RetVal rv;
	if (!consume(IF)) return 0;
	if (!consume(LPAR)) tkerr(crtToken, "lipsa ( dupa if");
	if (!expr(&rv)) tkerr(crtToken, "conditia din if nu are o forma valida");
	ATIP_START
		if (rv.type.typeBase == TB_STRUCT)
			tkerr(crtToken, "a structure cannot be logically tested");
	ATIP_END
	if (!consume(RPAR)) tkerr(crtToken, "expresia conditie din if nu are o forma valida sau paranteze dezechilibrate");
	if (!stm()) tkerr(crtToken, "corpul lui if nu are o forma corespunzatoare");
	if (consume(ELSE)) {
		if (!stm()) tkerr(crtToken,"corpul lui else nu are o forma corespunzatoare");
	}
	return 1;
}
static int _whileStm() {
	RetVal rv;
	if (!consume(WHILE)) return 0;
	if (!consume(LPAR)) tkerr(crtToken, "sintaxa invalida, lipsa ( dupa while");
	if (!expr(&rv)) tkerr(crtToken, "Erpresie invalida in conditia din while");
	ATIP_START
		if (rv.type.typeBase == TB_STRUCT)
			tkerr(crtToken, "a structure cannot be logically tested");
	ATIP_END
	if (!consume(RPAR)) tkerr(crtToken, "sintaxa invalida, expresie invalida in while sau paranteze rotunde dezechilibrate in while");
	if (!stm()) tkerr(crtToken, "corpul while are o forma necorespunzatoare");
	return 1;
}
static int _forStm() {
	RetVal rv1, rv2, rv3;
	if (!consume(FOR)) return 0;
	if (!consume(LPAR)) tkerr(crtToken, "sintaxa invalida, lipsa ( dupa for");
	expr(&rv1);
	if (!consume(SEMICOLON)) tkerr(crtToken, "sintaxa invalida, lipsa ; dupa prima expresie din for");
	if (expr(&rv2)) {
		ATIP_START
			if (rv2.type.typeBase == TB_STRUCT)
				tkerr(crtToken, "a structure cannot be logically tested");
		ATIP_END
	}
	if (!consume(SEMICOLON)) tkerr(crtToken, "sintaxa invalida, lipsa ; dupa a doua expresie din for");
	expr(&rv3);
	if (!consume(RPAR)) tkerr(crtToken, "sintaxa invalida, expresie invalida sau paranteze rotunde dezechilibrate in for");
	if (!stm()) tkerr(crtToken, "sintaxa invalida in corpul for");
	return 1;
}
static int _breakStm() {
	if (!consume(BREAK)) return 0;
	if (!consume(SEMICOLON)) tkerr(crtToken, "lipsa ; dupa break");
	return 1;
}
static int _returnStm() {
	RetVal rv;	
	if (!consume(RETURN)) return 0;
	expr(&rv);
	ATIP_START
		if (crtFunc->type.typeBase == TB_VOID)
			tkerr(crtToken, "a void function cannot return a value");
		cast(&crtFunc->type, &rv.type);
	ATIP_END
	if (!consume(SEMICOLON)) tkerr(crtToken, "lipseste ; dupa intructiunea");
	return 1;
}
static int _exprStm() {
	RetVal rv;
	if (expr(&rv)) {
		if (consume(SEMICOLON))
			return 1; 
		else 
			tkerr(crtToken, "lipseste ; dupa expresie");
	}
	return consume(SEMICOLON);
}


int stm() {
	if (stmCompound()) return 1;
	if (_ifStm()) return 1;
	if (_whileStm()) return 1;
	if (_forStm()) return 1;
	if (_breakStm()) return 1;
	if (_returnStm()) return 1;
	if (_exprStm()) return 1;
	return 0;
}

//stmCompound: LACC ( declVar | stm )* RACC ;
int stmCompound() {
	Token* start = crtToken;
	Symbol* crtSymbol = symbols.end[-1];


	if (consume(LACC)) {
		crtDepth++;
		while (1) {
			if (declVar()) continue;
			else if (stm()) continue;
			else break;
		}
		if (consume(RACC)) {
			crtDepth--;
			deleteSymbolsAfter(&symbols, crtSymbol);
			return 1;
		}
		else tkerr(crtToken,"acoladele nu sunt bine imperecheate");
	}

	crtToken = start;
	return 0;
}

int expr(RetVal* rv) {
	return exprAssign(rv);
}

//exprAssign: exprUnary ASSIGN exprAssign | exprOr ;
int exprAssign(RetVal* rv) {
	Token* start = crtToken;
	RetVal rve;
	if (exprUnary(rv)) {
		if (consume(ASSIGN)) {
			if (exprAssign(&rve)) {
				ATIP_START
					if (!rv->isLVal)tkerr(crtToken, "cannot assign to a non-lval");
					if (rv->type.nElements > -1 || rve.type.nElements > -1)
						tkerr(crtToken, "the arrays cannot be assigned");
					cast(&rv->type, &rve.type);
					rv->isCtVal = rv->isLVal = 0;
				ATIP_END
				return 1;
			}
			tkerr(crtToken, "Atribuire invalida");
		}
		crtToken = start;
	}
	return exprOr(rv);
}

#define SINTAXERR "Expresie invalida. Eroare de sintaxa"
/*	exprOr:rv = exprOr:rv OR exprAnd:rve | exprAnd:rv

	exprOr:rv = exprAnd:rve exprOr':rv
	exprOr':rv = OR exprAnd:rve exprOr':rv | eps	
			=> exprOrP = (OR exprAnd)* => exprOr:rv = exprAnd:rv (OR exprAnd:rve{})*

	cu mentionarea ca asta este valabila doar in cazul in care A' = a A' | eps <=> A' = a* daca nu gresesc
	da cred ca asta ar putea fi echivalent cu [ exprOr = exprAnd (OR exprAnd)*; ]
fara aceasta menriune codul ar fi:
int exprOrP(){
	if(consume(OR)){
		if(!exprAnd()) tkerr(crtToken,"Expresie invalida");
		if(!exprOrP()) tkerr(crtTiken,"Expresie invalida");
	}
	return 1;
}
int expOr(){
	if(!exprAnd()) return 0;
	if(!exprOrP()) tkerr(crtToken, "expresie invalida");
	return 1;

	  if(rv->type.typeBase==TB_STRUCT||rve.type.typeBase==TB_STRUCT)
				tkerr(crtTk,"a structure cannot be logically tested");
		rv->type=createType(TB_INT,-1);
		rv->isCtVal=rv->isLVal=0;
*/
int exprOr(RetVal* rv){
	RetVal rve;
	if (!exprAnd(rv)) return 0;
	while (1) {
		if (consume(OR)) {
			if (exprAnd(&rve)) {
				ATIP_START
					if (rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT)
						tkerr(crtToken, "a structure cannot be logically tested");
					rv->type = createType(TB_INT, -1);
					rv->isCtVal = rv->isLVal = 0;
				ATIP_END
			}
			else
				tkerr(crtToken, SINTAXERR);
		}
		else break;
	}
	return 1;
}

/*exprAnd:rv = exprAnd:rv AND exprEq:rve | exprEq:rv  ==>  exprAnd = exprEq (AND exprEq)* ;  */
int exprAnd(RetVal* rv) {
	RetVal rve;
	if (!exprEq(rv)) return 0;
	while (1) {
		if (consume(AND)) {
			if (exprEq(&rve)) {
				ATIP_START
					if (rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT)
						tkerr(crtToken, "a structure cannot be logically tested");
					rv->type = createType(TB_INT, -1);
					rv->isCtVal = rv->isLVal = 0;
				ATIP_END
			}
			else
				tkerr(crtToken, SINTAXERR);
		}
		else break;
	}
	return 1;
}

//exprEq: exprEq ( EQUAL | NOTEQ ) exprRel | exprRel  ==> exprEq: exprRel ((EQUAL|NOTEQ) exprRel)*;
int exprEq(RetVal* rv){
	RetVal rve;
	if (!exprRel(rv)) return 0;
	while (1) {
		if (consume(EQUAL) || consume(NOTEQ)) {
			if (exprRel(&rve)) {
				ATIP_START
					if (rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT)
						tkerr(crtToken, "a structure cannot be compared");
					rv->type = createType(TB_INT, -1);
					rv->isCtVal = rv->isLVal = 0;
				ATIP_END
			}
			else
				tkerr(crtToken, SINTAXERR);
		}
		else break;
	}
	return 1;
}

//exprRel: exprRel ( LESS | LESSEQ | GREATER | GREATEREQ ) exprAdd | exprAdd  => exprAdd ((||||) exprAdd)*;
int exprRel(RetVal* rv){
	RetVal rve;
	if (!exprAdd(rv)) return 0;
	while (1) {
		if (consume(LESS) || consume(LESSEQ) || consume(GREATER) || consume(GREATEREQ)) {
			if (exprAdd(&rve)) {
				ATIP_START
					if (rv->type.nElements > -1 || rve.type.nElements > -1)
						tkerr(crtToken, "an array cannot be compared");
					if (rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT)
						tkerr(crtToken, "a structure cannot be compared");
					rv->type = createType(TB_INT, -1);
					rv->isCtVal = rv->isLVal = 0;
				ATIP_END
			}else
				tkerr(crtToken, SINTAXERR);
		}
		else break;
	}
	return 1;
}

//exprAdd: exprAdd ( ADD | SUB ) exprMul | exprMul ; ==>  exprAdd = exprMul ((ADD|SUB) exprMul)*;
int exprAdd(RetVal* rv){
	RetVal rve;
	if (!exprMul(rv)) return 0;
	while (1) {
		if (consume(ADD) || consume(SUB)) {
			if (exprMul(&rve)) {
				ATIP_START
					if (rv->type.nElements > -1 || rve.type.nElements > -1)
						tkerr(crtToken, "an array cannot be added or subtracted");
					if (rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT)
						tkerr(crtToken, "a structure cannot be added or subtracted");
					rv->type = getArithType(&rv->type, &rve.type);
					rv->isCtVal = rv->isLVal = 0;
				ATIP_END
			}
			else
				tkerr(crtToken, SINTAXERR);
		}
		else break;
	}
	return 1;
}

//exprMul: exprMul ( MUL | DIV ) exprCast | exprCast;  ==> exprMul: exprCast ((MUL|DIV) exprCast)*; 
int exprMul(RetVal* rv){
	RetVal rve;
	if (!exprCast(rv)) return 0;
	while (1) {
		if (consume(MUL) || consume(DIV)) {
			if (!exprCast(&rve)) tkerr(crtToken, SINTAXERR);
			ATIP_START
				if (rv->type.nElements > -1 || rve.type.nElements > -1)
					tkerr(crtToken, "an array cannot be multiplied or divided");
				if (rv->type.typeBase == TB_STRUCT || rve.type.typeBase == TB_STRUCT)
					tkerr(crtToken, "a structure cannot be multiplied or divided");
				rv->type = getArithType(&rv->type, &rve.type);
				rv->isCtVal = rv->isLVal = 0;
			ATIP_END
		}
		else break;
	}
	return 1;
}

//exprCast: LPAR typeName RPAR exprCast | exprUnary ;
int exprCast(RetVal* rv) {
	Token* start = crtToken;
	Type t;
	RetVal rve;
	if (consume(LPAR)) {
		if (typeName(&t)) {
			if (consume(LPAR)) {
				if (exprCast(&rve)) {
					ATIP_START
						cast(&t, &rve.type);
						rv->type = t;
						rv->isCtVal = rv->isLVal = 0;
					ATIP_END
					return 1;
				}
			}
			else tkerr(crtToken, "tip declarat sub o forma invlida sau lipsa paranteza");
		}
		else tkerr(crtToken, "nume de tip invalid");
	}
	return exprUnary(rv);
}

//exprUnary: ( SUB | NOT ) exprUnary | exprPostfix ;
int exprUnary(RetVal* rv) {
	Token* tkop;

	if (consume(SUB) || consume(NOT)) {
		tkop = consumedTk;
		if (exprUnary(rv)) {
			ATIP_START
				if (tkop->code == SUB) {
					if (rv->type.nElements >= 0)tkerr(crtToken, "unary '-' cannot be applied to an array");
					if (rv->type.typeBase == TB_STRUCT)
						tkerr(crtToken, "unary '-' cannot be applied to a struct");
				}
				else {  // NOT
					if (rv->type.typeBase == TB_STRUCT)tkerr(crtToken, "'!' cannot be applied to a struct");
					rv->type = createType(TB_INT, -1);
				}
				rv->isCtVal = rv->isLVal = 0;
			ATIP_END
			return 1;
		}
		else tkerr(crtToken,"expresie unara invalida, - sau ! care nu este urmat de o expresie postfix");
	}
	return exprPostfix(rv);
}

/*exprPostfix:rv = exprPostfix:rv LBRACKET expr:rve {} RBRACKET
           | exprPostfix DOT ID:tkName {} 
           | exprPrimary:rv ;		
		   
exprPostfix:rv : exprPrimary:rv exprPostfix':rv;
exprPostfix':rv : LBRACKET expr:rve {1} RBRACKET exprPostfix':rv | DOT ID {2} exprPostfix' | eps;
		   
		   {1}  if(rv->type.nElements<0)tkerr(crtTk,"only an array can be indexed");
            Type typeInt=createType(TB_INT,-1);
            cast(&typeInt,&rve.type);
            rv->type=rv->type;
            rv->type.nElements=-1;
            rv->isLVal=1;
            rv->isCtVal=0;

			{2} Symbol      *sStruct=rv->type.s;
            Symbol      *sMember=findSymbol(&sStruct->members,tkName->text);
            if(!sMember)
                tkerr(crtTk,"struct %s does not have a member %s",sStruct->name,tkName->text);
            rv->type=sMember->type;
            rv->isLVal=1;
            rv->isCtVal=0;
		   */

int exprPostfixP(RetVal* rv) {
	RetVal rve;
	if (consume(LBRACKET)) {
		if (!expr(&rve)) tkerr(crtToken, "Expresie invalida intre paranteze");
		ATIP_START
			if (rv->type.nElements < 0) tkerr(crtToken, "only an array can be indexed");
			Type typeInt = createType(TB_INT, -1);
			cast(&typeInt, &rve.type);
			rv->type = rv->type;
			rv->type.nElements = -1;
			rv->isLVal = 1;
			rv->isCtVal = 0;
		ATIP_END
		if (!consume(RBRACKET)) tkerr(crtToken, "expresie invalida intre paranteze sau paranteze patrate dezechilibrate");
		exprPostfixP(rv);
	}
	else if (consume(DOT)) {
		Token *tkName;
		if (!consume(ID)) tkerr(crtToken, SINTAXERR);
		ATIP_START
			tkName = consumedTk;
			Symbol* sStruct = rv->type.struct_elems;
			Symbol* sMember = findSymbol(&sStruct->members, tkName->text);
			if (!sMember)
				tkerr(crtToken, "struct %s does not have a member %s", sStruct->name, tkName->text);
			else {
				rv->type = sMember->type;
				rv->isLVal = 1;
				rv->isCtVal = 0;
			}
		ATIP_END
		exprPostfixP(rv);
	}
	return 1; // | eps
}

int exprPostfix(RetVal* rv) {
	if (!exprPrimary(rv)) return 0;
	exprPostfixP(rv); // nu are rost sa verific findca are epsilon in constructie
	return 1;
}

/*exprPrimary: ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
           | CT_INT
           | CT_REAL 
           | CT_CHAR 
           | CT_STRING 
           | LPAR expr RPAR ;		*/
int exprPrimary(RetVal* rv) {
	Token* tkName;
	tkName = crtToken;
	RetVal arg;
	Symbol* struct_elems;

	if (consume(ID)) {
		ATIP_START
			struct_elems = findSymbol(&symbols, tkName->text);
		if (!struct_elems){
			tkerr(crtToken, "undefined symbol %s", tkName->text);
			return;
		}
			else {
				rv->type = struct_elems->type;
				rv->isCtVal = 0;
				rv->isLVal = 1;
			}
		ATIP_END
		if (consume(LPAR)) {
			ATIP_START
				Symbol** crtDefArg = struct_elems->args.begin;
				if (struct_elems->cls != CLS_FUNC && struct_elems->cls != CLS_EXTFUNC)
					tkerr(crtToken, "call of the non-function %s", tkName->text);
			ATIP_END
			if (expr(&arg)) {
				ATIP_START
					if (crtDefArg == struct_elems->args.end)tkerr(crtToken, "too many arguments in call");
					cast(&(*crtDefArg)->type, &arg.type);
					crtDefArg++;
				ATIP_END
				while (1) {
					if (consume(COMMA)) {
						if (!expr(&arg)) tkerr(crtToken, "expresie invalida");
						else {
							ATIP_START
								if (crtDefArg == struct_elems->args.end)tkerr(crtToken, "too many arguments in call");
								cast(&(*crtDefArg)->type, &arg.type);
								crtDefArg++;
							ATIP_END
						}
					}
					else break;
				}
			}
			if (!consume(RPAR)) tkerr(crtToken, "expresie invalida inainte de ) sau paranteze dezechilibrate");
			else {
				ATIP_START
					if (crtDefArg != struct_elems->args.end) tkerr(crtToken, "too few arguments in call");
					rv->type = struct_elems->type;
					rv->isCtVal = rv->isLVal = 0;
				ATIP_END
			}
		}
		else {
			ATIP_START
				if (struct_elems && (struct_elems->cls == CLS_FUNC || struct_elems->cls == CLS_EXTFUNC))
					tkerr(crtToken, "missing call for function %s", tkName->text);
			ATIP_END
				
		}
		return 1;
	}
	tkName = crtToken;
	if (consume(CT_INT)) {
		ATIP_START
			rv->type = createType(TB_INT, -1);
			rv->ctVal.i = tkName->i;
			rv->isCtVal = 1; 
			rv->isLVal = 0;
		ATIP_END
		return 1;
	}
	if (consume(CT_REAL)) {
		ATIP_START
			rv->type = createType(TB_DOUBLE, -1);
			rv->ctVal.d = tkName->r;
			rv->isCtVal = 1; 
			rv->isLVal = 0;
		ATIP_END
		return 1;
	}
	if (consume(CT_CHAR)) {
		ATIP_START
			rv->type = createType(TB_CHAR, -1);
			rv->ctVal.i = tkName->i;
			rv->isCtVal = 1; 
			rv->isLVal = 0;
		ATIP_END
		return 1;
	}
	if (consume(CT_STRING)) {
		ATIP_START
			rv->type = createType(TB_CHAR, 0);
			rv->ctVal.str = tkName->text;
			rv->isCtVal = 1; 
			rv->isLVal = 0;
		ATIP_END
		return 1;
	}
	if (consume(LPAR)) {
		if (!expr(rv)) tkerr(crtToken, "Expresie invalida sau absenta");	
		if (!consume(RPAR)) tkerr(crtToken, "Expresie invalida intre paranteze rotunde sau paranteze dezechilibrate");
	}

	return 0;
}