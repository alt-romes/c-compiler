#ifndef _ENVIRONMENT_H
#define _ENVIRONMENT_H

#define DEFAULT_ENVIRONMENT_SIZE 10

struct association {
    char* id;
    void* val;
};

typedef struct environment {
    struct environment* parent;
    struct association* associations;
    int size;
} environment_t;

environment_t* newEnvironment();

environment_t* beginScope(environment_t* e);
environment_t* endScope(environment_t* e);

void assoc(environment_t* e, char* id, void* val);

void* find(environment_t* e, char* id);

#endif
