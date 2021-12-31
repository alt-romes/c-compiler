#ifndef _TYPECHECK_H
#define _TYPECHECK_H

#include "ast.h"

type_t typecheck(struct node* node, struct environment* e);

#endif
