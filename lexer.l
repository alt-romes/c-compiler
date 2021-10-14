%{
#include "y.tab.h"
extern int yylval;
%}

%%

"+"       return PLUS;
"-"       return MINUS;
"*"       return MUL;
"/"       return DIV;
"("       return LPAR;
")"       return RPAR;
[0-9]+    { yylval = atoi(yytext); return Num; }
[ \t]     ;
"\n"      return EL;

%%

int yywrap() {
    return 1;
}
