%{
#include "ast.h"
#include "y.tab.h"
extern YYSTYPE yylval;
%}

%%

"int"     return _INT;
"short"   return _SHORT;
"char"    return _CHAR;
"const"   return _CONST;
[ \t\n]+  ;
[0-9]+    { yylval.int_v = atoi(yytext); return _NUM; }
[a-zA-Z][a-zA-Z0-9]*  { yylval.string_v = strdup(yytext); return _IDENTIFIER; }
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
