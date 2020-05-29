#ifndef _ANSIN_H__INCLUDED_
#define _ANSIN_H__INCLUDED_

extern int AnalizorSintactic(Token* fstTok);
#define ANSIN(tk) AnalizorSintactic(tk)

#endif
