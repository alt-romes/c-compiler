%{

#include <stdio.h>
#include <stdint.h>
#include "environment.h"
#include "ast.h"

int yylex(); // defined by lex
void yyerror();

%}

%union {
    int int_v;
    char* string_v;
    struct node* node_v;
    struct environment* environment_v;
    struct association association_v;
}

%token _INT
%token _Num
%token _IDENTIFIER

%type <int_v> _Num
%type <string_v> _IDENTIFIER declarator direct_declarator // string is malloc'd and needs to be freed
%type <node_v> init initializer statement_list exp term fact compound_statement
%type <environment_v> declaration_list declaration init_declarator_list
%type <association_v> init_declarator

%parse-param {node_t** root}

%start init

%%

exp
   : term                                       { $$ = $1; }
   | term '+' exp                               { $$ = create_node2(ADD, $1, $3); }
   | term '-' exp                               { $$ = create_node2(SUB, $1, $3); }

term
    : fact                                      { $$ = $1; }
    | fact '*' term                             { $$ = create_node2(MUL, $1, $3); }
    | fact '/' term                             { $$ = create_node2(DIV, $1, $3); }

fact
    : compound_statement                        { $$ = $1; }
    | _Num                                      { $$ = create_node_literal(NUM, (void*)(intptr_t)$1); }
    | _IDENTIFIER                               { $$ = create_node_literal(ID, $1); }
    | '(' exp ')'                               { $$ = $2; }
    | '-' fact                                  { $$ = create_node1(UMINUS, $2); }


init
    : statement_list                            { *root = $1; }
    | compound_statement                        { *root = $1; }

compound_statement  // also known as "block"
    : '{' statement_list '}'                    { $$ = $2; }
    | '{' declaration_list statement_list '}'   { $$ = create_node_block(BLOCK, $2, $3); }

declaration_list
    : declaration                               { $$ = $1; }
    | declaration_list declaration              { $$ = merge_environment($2, $1); }

declaration
    : _INT init_declarator_list ';'             { $$ = $2; }

init_declarator_list
    : init_declarator                           { $$ = assoc(newEnvironment(), $1.id, $1.val); }
    | init_declarator_list ',' init_declarator  { $$ = assoc($1, $3.id, $3.val); }

init_declarator
    : declarator                                { $$ = (struct association){ .id = $1, .val = NULL }; }
    | declarator '=' initializer                { $$ = (struct association){ .id = $1, .val = (void*)$3 }; }

declarator
    : direct_declarator                         { $$ = $1; }

direct_declarator
    : _IDENTIFIER                               { $$ = $1; }

initializer
    : exp                                       { $$ = $1; }

statement_list
    : exp ';'                                   { $$ = $1; }

%%

void yyerror(const char* str) {
    fprintf(stderr,"Syntax error: %s\n",str);
}

