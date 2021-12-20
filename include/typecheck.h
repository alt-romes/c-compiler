#ifndef _TYPECHECK_H
#define _TYPECHECK_H

#include "ast.h"

enum type typecheck(struct node* node, struct environment* e);

#endif
