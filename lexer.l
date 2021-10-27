%{
#include "y.tab.h"
extern YYSTYPE yylval;
%}

%%

"+"       return _ADD;
"-"       return _SUB;
"*"       return _MUL;
"/"       return _DIV;
"("       return _LPAR;
")"       return _RPAR;
[0-9]+    { yylval = atoi(yytext); return _Num; }
[ \t]     ;
"\n"      return _EL;

%%

int yywrap() {
    return 1;
}
