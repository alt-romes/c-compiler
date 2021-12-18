%{
#include "ast.h"
#include "y.tab.h"
extern YYSTYPE yylval;
%}

%%

"int"     return _INT;
"short"   return _SHORT;
"char"    return _CHAR;
"unsigned"   return _UNSIGNED;
"signed"   return _SIGNED;
"const"   return _CONST;
"true"    { yylval.int_v = 1; return _BOOL; }
"false"   { yylval.int_v = 0; return _BOOL; }
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
