#ifndef _DCPU_IR
#define _DCPU_IR

#include "ast.h"

void dcpu_print();
void dcpu_free();

void emit_num(int num_value);
void emit_add();
void emit_sub();
void emit_mul();
void emit_div();
void emit_uminus();


#endif