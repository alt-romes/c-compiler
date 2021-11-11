#include "ast.h"
#include "parse_utils.h"

int yyparse(); // defined by yacc

node_t* parse_root() {
    node_t* root = 0;
    yyparse(&root);
    return root;
}
