%{

#include <stdio.h>
#include "ast.h"

#define YYSTYPE node_t*

int yylex(); // defined by lex
void yyerror();

extern node_t* _root;

%}

%token _Id
%token _Num
%token _ADD
%token _SUB
%token _MUL
%token _DIV
%token _LPAR
%token _RPAR
%token _EL

%start line

%%

line:
   exp _EL    { _root = $1; return 0; }

exp:
   term                { $$ = $1; }
   | term _ADD exp     { $$ = create_node2(ADD, $1, $3); }
   | term _SUB exp    { $$ = create_node2(SUB, $1, $3); }

term:
    fact                { $$ = $1; }
    | fact _MUL term     { $$ = create_node2(MUL, $1, $3); }     
    | fact _DIV term     { $$ = create_node2(DIV, $1, $3); }

fact:
    _Num                 { $$ = create_node_literal(NUM, &$1); }
    | _LPAR exp _RPAR     { $$ = $2; }
    | _SUB fact        { $$ = create_node1(UMINUS, $2); }

%%

void yyerror() {
    printf("Syntax error!\n");
}
