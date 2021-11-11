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
    environment_t* p = e->parent;

    free(e->associations);
    free(e);

    return p;
}

environment_t* assoc(environment_t* e, char* id, void* val) {
    if (!e->size % DEFAULT_ENVIRONMENT_SIZE) {
        struct association* new_associations = realloc(e->associations, (e->size+DEFAULT_ENVIRONMENT_SIZE)*sizeof(struct association));
        e->associations = new_associations;
    }
    e->associations[e->size].id = id;
    e->associations[e->size++].val = val;

    return e;
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

environment_t* merge_environment(environment_t* src, environment_t* dst) {

    for (int i = 0; i < src->size; i++)
        assoc(dst, src->associations[i].id, src->associations[i].val);

    free(src->associations);
    free(src);
    
    return dst;
}
