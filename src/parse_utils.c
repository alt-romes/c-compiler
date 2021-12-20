#include <stdlib.h>
#include "ast.h"
#include "parse_utils.h"

int yyparse(); // defined by yacc

node_t* parse_root() {
    node_t* root = NULL;
    yyparse(&root);
    return root;
}
