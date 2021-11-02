%{

#include <stdio.h>
#include <stdint.h>
#include "ast.h"

/* #define YYSTYPE node_t* */

int yylex(); // defined by lex
void yyerror();

extern node_t* _root;

%}

%union {
    int int_v;
    char* string_v;
    struct node* node_v;
}

%token _ADD
%token _SUB
%token _MUL
%token _DIV
%token _LPAR
%token _RPAR
%token _EL
%token _DEF
%token _EQ
%token _IN
%token _END
%token _Num
%token _Id

%type <int_v> _Num
%type <string_v> _Id // string is malloc'd and needs to be freed
%type <node_v> line exp term fact def

%start line

%%

line:
   exp _EL                              { _root = $1; return 0; }

exp:
   term                                 { $$ = $1; }
   | term _ADD exp                      { $$ = create_node2(ADD, $1, $3); }
   | term _SUB exp                      { $$ = create_node2(SUB, $1, $3); }

term:
    fact                                { $$ = $1; }
    | fact _MUL term                    { $$ = create_node2(MUL, $1, $3); }     
    | fact _DIV term                    { $$ = create_node2(DIV, $1, $3); }

fact:

    def                                 { $$ = $1; }
    | _Num                              { $$ = create_node_literal(NUM, (void*)(intptr_t)$1); }
    | _Id                               { $$ = create_node_literal(ID, $1); }
    | _LPAR exp _RPAR                   { $$ = $2; }
    | _SUB fact                         { $$ = create_node1(UMINUS, $2); }

def:
   _DEF _Id _EQ exp _IN exp _END        { $$ = create_node_def(DEF, $2, $4, $6); }

%%

void yyerror() {
    printf("Syntax error!\n");
}
