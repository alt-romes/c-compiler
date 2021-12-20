#ifndef _ENVIRONMENT_H
#define _ENVIRONMENT_H

#include <llvm-c/Core.h>
#include "types.h"

#define DEFAULT_ENVIRONMENT_SIZE 10

union association_v {
    struct node* node;
    enum type type;
    LLVMValueRef llvmref;
    int integer;
};

struct association {
    char* id;
    union association_v val;
};

typedef struct environment {
    struct environment* parent;
    struct association* associations;
    int size;
} environment_t;

environment_t* newEnvironment();

environment_t* beginScope(environment_t* e);
environment_t* endScope(environment_t* e);

environment_t* assoc(environment_t* e, char* id, union association_v val); // returns the environment passed as the first argument

union association_v find(environment_t* e, char* id);

#endif
