#include "ast.h"
#include "parse_utils.h"

int yyparse(); // defined by yacc

node_t* _root;

node_t* parse_root() {
    yyparse();
    return _root;
}
