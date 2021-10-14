%{
#include <stdio.h>
void yyerror();
extern int yylex();
%}
%token Id
%token Num
%token PLUS
%token MINUS
%token MUL
%token DIV
%token LPAR
%token RPAR
%token EL

%start line

%%

line:
   exp EL    { return $1; }

exp:
   term                 { $$ = $1; }
   | term PLUS exp     { $$ = $1 + $3; }
   | term MINUS exp    { $$ = $1 - $3; }

term:
    fact                { $$ = $1; }
    | fact MUL term     { $$ = $1 * $3; }     
    | fact DIV term     { $$ = $1 * $3; }

fact:
    Num                 { $$ = $1; }
    | LPAR exp RPAR     { $$ = $2; }
    | MINUS fact        { $$ = -$2; }

%%

int main(void) {
    printf("Enter expression:\n");
    int n = yyparse();
    printf("%d\n", n);
    return 0;
}

void yyerror() {
    printf("Syntax error!\n");
}
