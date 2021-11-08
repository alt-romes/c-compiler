#include "dcpuIR.h"

#include <stdlib.h>
#include<stdio.h>
#include<string.h>
#include <stdarg.h>

typedef  struct _inode inode;

struct _inode {

    char inst[16];
    inode* next;
};

inode* first = NULL;
inode* last = NULL;

void newInstruction(const char* inst_format, ...) {

    va_list args;
    va_start(args, inst_format);

    inode* in = malloc(sizeof(inode));
    vsprintf(in->inst, inst_format, args);

    if(last == NULL)
        first = in;
    else
        last->next = in; // TODO: Initialized va_list 'args' is leaked
                         //       [clang-analyzer-valist.Unterminated]

    last = in;
}

void dcpu_print() {

    inode* in = first;
    while(in != NULL) {

        printf("%s", in->inst);
        in = in->next;
    }

    printf("pop RA\n");
    printf("hlt\n");
}

void dcpu_free() {

    inode* in = first;
    while(in != NULL) {

        inode* prev = in;
        in = in->next;
        free(prev);
    }
}

void emit_num(int num_value){

    newInstruction("psh $%d\n", num_value);
}
void emit_add(){

    newInstruction("pop RB\n");
    newInstruction("pop RC\n");
    newInstruction("add RB RC\n");
    newInstruction("lod ACR RB\n");
    newInstruction("psh RB\n");
}
void emit_sub(){

	newInstruction("pop RB\n");
    newInstruction("pop RC\n");
    newInstruction("sub RB RC\n");
    newInstruction("lod ACR RB\n");
    newInstruction("psh RB\n");
}
void emit_mul(){
	
    newInstruction("pop RB\n"); // a
    newInstruction("pop RC\n"); // b
    newInstruction("lod $0 RD\n"); // res
    newInstruction("loop_start:\n");
    newInstruction("add RD RB\n"); // res += a
    newInstruction("lod ACR RD\n");
    newInstruction("dec RC\n"); // b--
    newInstruction("jmpz loop_end\n");
    newInstruction("lod ACR RC\n");
    newInstruction("jmp loop_start\n");
    newInstruction("loop_end:\n");
    newInstruction("psh RD\n");
}
void emit_div(){
	
}
void emit_uminus(){

    newInstruction("pop RB\n");
    newInstruction("neg RB\n");
    newInstruction("psh RB\n");
}
