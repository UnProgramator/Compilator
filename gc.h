#ifndef _ALEX_H__INCLUDED_
#define _ALEX_H__INCLUDED_

#include "atip.h"
#include "adom.h"
#include "mv.h"

extern int sizeArgs, offset;

extern int typeBaseSize(Type* type);
extern int typeFullSize(Type* type);
extern int typeArgSize(Type* type);

#endif