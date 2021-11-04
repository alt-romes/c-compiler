#ifndef _DCPU_CODE_BLOCK
#define _DCPU_CODE_BLOCK

#include "ast.h"

void printCodeBlock();
void freeCodeBlock();

void emit_num(int num_value);
void emit_add();
void emit_sub();
void emit_mul();
void emit_div();
void emit_uminus();


#endif