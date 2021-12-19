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
"=="      return _EQ_OP;
"!="      return _NE_OP;
">="      return _GE_OP;
"<="      return _LE_OP;
"&&"      return _AND_OP;
"||"      return _OR_OP;
"<<"      return _LEFT_OP;
">>"      return _RIGHT_OP;
[ \t\n]+  ;
[0-9]+    { yylval.int_v = atoi(yytext); return _NUM; }
[a-zA-Z][a-zA-Z0-9]*  { yylval.string_v = strdup(yytext); return _IDENTIFIER; }
"+"       return '+';
"-"       return '-';
"!"       return '!';
"*"       return '*';
"/"       return '/';
"="       return '=';
";"       return ';';
","       return ',';
"("       return '(';
")"       return ')';
"{"       return '{';
"}"       return '}';
"<"       return '<';
">"       return '>';
"&"       return '&';
"|"       return '|';
"^"       return '^';

%%

int yywrap() {
    return 1;
}
