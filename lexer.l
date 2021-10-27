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
"def"     return _DEF;
"="       return _EQ;
"in"      return _IN;
"end"     return _END;
[0-9]+    { yylval.int_v = atoi(yytext); return _Num; }
[a-zA-Z][a-zA-Z0-9]*    { yylval.string_v = yytext; return _Id; }
[ \t]     ;
"\n"      return _EL;

%%

int yywrap() {
    return 1;
}
