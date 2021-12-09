%{

#include <stdio.h>
#include <stdint.h>
#include "ast.h"

int yylex(); // defined by lex
void yyerror();

%}

%union {
    int int_v;
    char* string_v;
    struct node* node_v;
    struct declaration_list* declaration_list_v;
    struct declaration declaration_v;
    struct declaration_specifiers declaration_specifiers_v;
}

%token _INT
%token _CONST
%token _NUM
%token _IDENTIFIER

%type <int_v> _NUM type_specifier type_qualifier
%type <string_v> _IDENTIFIER declarator direct_declarator // string is malloc'd and needs to be freed by the ast destructor
%type <node_v> init initializer statement_list exp term fact compound_statement
%type <declaration_list_v> declaration_list declaration init_declarator_list
%type <declaration_v> init_declarator
%type <declaration_specifiers_v> declaration_specifiers

%parse-param {node_t** root}

%start init

%%

exp
   : term                                             { $$ = $1; }
   | term '+' exp                                     { $$ = create_node2(ADD, $1, $3); }
   | term '-' exp                                     { $$ = create_node2(SUB, $1, $3); }

term
    : fact                                            { $$ = $1; }
    | fact '*' term                                   { $$ = create_node2(MUL, $1, $3); }
    | fact '/' term                                   { $$ = create_node2(DIV, $1, $3); }

fact
    : compound_statement                              { $$ = $1; }
    | _NUM                                            { $$ = create_node_literal(NUM, (void*)(intptr_t)$1); }
    | _IDENTIFIER                                     { $$ = create_node_literal(ID, $1); }
    | '(' exp ')'                                     { $$ = $2; }
    | '-' fact                                        { $$ = create_node1(UMINUS, $2); }


init
    : statement_list                                  { *root = $1; }
    | compound_statement                              { *root = $1; }

compound_statement  // also known as "block"
    : '{' statement_list '}'                          { $$ = $2; }
    | '{' declaration_list statement_list '}'         { $$ = create_node_block(BLOCK, $2, $3); }

declaration_list
    : declaration                                     { $$ = $1; }
    | declaration_list declaration                    { $$ = declaration_list_merge($2, $1); }

declaration
    : declaration_specifiers init_declarator_list ';' { $$ = add_declaration_specifiers($1, $2); }

declaration_specifiers
    : type_specifier                                  { $$ = (struct declaration_specifiers){ .tq = -1, .ts = $1}; }
    | type_qualifier type_specifier                   { $$ = (struct declaration_specifiers){ .tq = $1, .ts = $2}; }

type_qualifier
    : _CONST                                          { $$ = CONST; }

type_specifier
    : _INT                                            { $$ = INT; }

init_declarator_list
    : init_declarator                                 { $$ = declaration_list_assoc(create_declaration_list(), $1); }
    | init_declarator_list ',' init_declarator        { $$ = declaration_list_assoc($1, $3); }

init_declarator
    : declarator                                      { $$ = (struct declaration){ .id = $1, EMPTY_DEC_SPECS, .node = NULL }; }
    | declarator '=' initializer                      { $$ = (struct declaration){ .id = $1, EMPTY_DEC_SPECS, .node = $3   }; }

declarator
    : direct_declarator                               { $$ = $1; }

direct_declarator
    : _IDENTIFIER                                     { $$ = $1; }

initializer
    : exp                                             { $$ = $1; }

statement_list
    : exp ';'                                         { $$ = $1; }

%%

void yyerror(const char* str) {
    fprintf(stderr,"Syntax error: %s\n",str);
}

