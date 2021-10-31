#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "environment.h"

environment_t* newEnvironment() { 
    environment_t* e = malloc(sizeof(environment_t));
    e->parent = NULL;
    return e;
}

environment_t* beginScope(environment_t* e) {

    environment_t* new_e = newEnvironment();
    new_e->parent = e;
    return new_e;
}

environment_t* endScope(environment_t* e) {
    return e->parent;
}

void assoc(environment_t* e, char* id, void* val) {
    if (!e->size % DEFAULT_ENVIRONMENT_SIZE) {
        struct association* new_associations = malloc((e->size+DEFAULT_ENVIRONMENT_SIZE)*sizeof(struct {char* id; void* val;} *));
        for (int i=0; i<e->size; i++)
            new_associations[i] = e->associations[i];
        free(e->associations);
        e->associations = new_associations;
    }
    e->associations[e->size].id = id;
    e->associations[e->size++].val = val;
}

void* find(environment_t* e, char* id) {
    for (int i=0; i<e->size; i++) {
        if (!strcmp(e->associations[i].id, id))
            return e->associations[i].val;
    } 
    if (e->parent != NULL)
        return find(e->parent, id);
    else {
        fprintf(stderr, "Identifier (%s) not found in the context!\n", id);
        exit(1);
    }
}