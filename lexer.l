%{
#include "environment.h"
#include "y.tab.h"
extern YYSTYPE yylval;
%}

%%

"int"     return _INT;
[ \t\n]+  ;
[0-9]+    { yylval.int_v = atoi(yytext); return _Num; }
[a-zA-Z][a-zA-Z0-9]*    { yylval.string_v = strdup(yytext); return _IDENTIFIER; }
"+"       return '+';
"-"       return '-';
"*"       return '*';
"/"       return '/';
"="       return '=';
";"       return ';';
","       return ',';
"("       return '(';
")"       return ')';
"{"       return '{';
"}"       return '}';

%%

int yywrap() {
    return 1;
}
