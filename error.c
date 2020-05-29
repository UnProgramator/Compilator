#include "token.h"
#include <stdio.h>
#include <stdarg.h>

extern void exit(int);

void err(const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "error: ");
	vfprintf(stderr, fmt, va);
	fputc('\n', stderr);
	va_end(va);
	exit(-1);
}

void tkerr(const Token* tk, const char* fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	fprintf(stderr, "error in tocken %d at line %d: ", tk->code, tk->line);
	vfprintf(stderr, fmt, va);
	fputc('\n', stderr);
	va_end(va);
	exit(-1);
}