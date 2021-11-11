#ifndef _ENVIRONMENT_H
#define _ENVIRONMENT_H

#define DEFAULT_ENVIRONMENT_SIZE 10

typedef struct association {
    char* id;
    void* val;
} association;

typedef struct environment {
    struct environment* parent;
    struct association* associations;
    int size;
} environment_t;

environment_t* newEnvironment();

environment_t* beginScope(environment_t* e);
environment_t* endScope(environment_t* e);

environment_t* assoc(environment_t* e, char* id, void* val); // returns the environment passed as the first argument

void* find(environment_t* e, char* id);

environment_t* merge_environment(environment_t* src, environment_t* dst); // merge two environments by copying all associations from src to dst, freeing src, and keeping dst's parent

#endif
