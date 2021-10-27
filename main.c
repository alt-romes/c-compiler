#include <stdio.h>
#include "ast.h"
/* #include "parser.h" */

int yyparse(); // defined by yacc

node_t* root;

int main(int argc, char *argv[]) {

    printf("Enter expression:\n");

    yyparse();
    printf("Root type: %d\n", root->type);

    free_ast(root);

    return 0;
}
