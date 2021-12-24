%{

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "ast.h"

int yylex(); // defined by lex
void yyerror();

%}

%union {
    int int_v;
    char* string_v;
    struct node* node_v;
    node_type_t node_type_v;
    struct declaration_list* declaration_list_v;
    struct statement_list* statement_list_v;
    struct declaration declaration_v;
    enum type declaration_specifiers_v;
}

%token _INT _SHORT _CHAR _UNSIGNED _SIGNED
%token _CONST
%token _NUM
%token _IDENTIFIER
%token _EQ_OP _NE_OP _LE_OP _GE_OP _OR_OP _AND_OP _LEFT_OP _RIGHT_OP
%token _MUL_ASSIGN _DIV_ASSIGN _MOD_ASSIGN _ADD_ASSIGN _SUB_ASSIGN _LEFT_ASSIGN _RIGHT_ASSIGN _AND_ASSIGN _XOR_ASSIGN _OR_ASSIGN _INC_OP _DEC_OP
%token _RETURN _IF _ELSE

%type <int_v> _NUM type_specifier type_qualifier specifier_qualifier_list type_name
%type <string_v> _IDENTIFIER declarator direct_declarator // string is malloc'd and needs to be freed by the ast destructor
%type <node_v> initializer compound_statement function_definition expression assignment_expression conditional_expression logical_or_expression logical_and_expression inclusive_or_expression exclusive_or_expression and_expression equality_expression relational_expression shift_expression additive_expression multiplicative_expression cast_expression unary_expression postfix_expression primary_expression statement expression_statement jump_statement selection_statement
%type <declaration_list_v> declaration_list declaration init_declarator_list
%type <statement_list_v> statement_list
%type <declaration_v> init_declarator
%type <declaration_specifiers_v> declaration_specifiers
%type <node_type_v> unary_operator assignment_operator

%parse-param {node_t** root}

%start init

%%

init
    : function_definition                             { *root = $1; }

function_definition
    // TODO: HOW TO HANDLE DECLARATION SPECIFIERS FOR FUNCTIONS?
    : declaration_specifiers declarator compound_statement { $$ = create_node_function(FUNCTION, $1, $2, $3); }

compound_statement // also known as "block"
    : '{' statement_list '}'                          { $$ = create_node_block(BLOCK, create_declaration_list() /* empty by default */ , $2); }
    | '{' declaration_list statement_list '}'         { $$ = create_node_block(BLOCK, $2, $3); }

declaration_list
    : declaration                                     { $$ = $1; }
    | declaration_list declaration                    { $$ = declaration_list_merge($2, $1); }

declaration
    : declaration_specifiers init_declarator_list ';' { $$ = add_declaration_specifiers($2, $1); }

declaration_specifiers
    : type_specifier                                  { $$ = (enum type) $1; }
    | type_specifier declaration_specifiers           { $$ = (enum type)($1 | $2); }
    | type_qualifier declaration_specifiers           { $$ = (enum type)($1 | $2); }

type_qualifier
    : _CONST                                          { $$ = CONST; }

type_specifier
    : _INT                                            { $$ = INT;       }
    | _SHORT                                          { $$ = SHORT;     }
    | _CHAR                                           { $$ = CHAR;      }
    | _UNSIGNED                                       { $$ = UNSIGNED;  }
    | _SIGNED                                         { $$ = 0;  }

init_declarator_list
    : init_declarator                                 { $$ = declaration_list_assoc(create_declaration_list(), $1); }
    | init_declarator_list ',' init_declarator        { $$ = declaration_list_assoc($1, $3); }

init_declarator
    : declarator                                      { $$ = (struct declaration){ .id = $1, EMPTY_DEC_SPECS, .node = NULL }; }
    | declarator '=' initializer                      { $$ = (struct declaration){ .id = $1, EMPTY_DEC_SPECS, .node = $3 }; }

declarator
    : direct_declarator                               { $$ = $1; }

direct_declarator
    : _IDENTIFIER                                     { $$ = $1; }

initializer
    : assignment_expression                                             { $$ = $1; }
    /* | '{' initializer_list '}' */
    /* | '{' initializer_list ',' '}' */

statement_list
	: statement { $$ = statement_list_add(create_statement_list(), $1); }
	| statement_list statement { $$ = statement_list_add($1, $2); }

statement
	/* : labeled_statement */
	: compound_statement { $$ = $1; }
	| expression_statement { $$ = $1; }
	| selection_statement { $$ = $1; }
	/* | iteration_statement */
	| jump_statement

expression_statement
	/* : ';'            { $$ = ???; }; // TODO: What to return here */
	: expression ';' { $$ = $1; }

expression
	: assignment_expression { $$ = $1; }
	| expression ',' assignment_expression { $$ = create_node2(SEQEXP, $1, $3); }

assignment_expression
    : conditional_expression { $$ = $1; }
    | unary_expression assignment_operator assignment_expression { $$ = create_node2($2, $1, $3); }

assignment_operator
    : '=' { $$ = ASSIGN; }
    | _MUL_ASSIGN { $$ = MUL_ASSIGN; }
	| _DIV_ASSIGN { $$ = DIV_ASSIGN; }
	| _MOD_ASSIGN { $$ = MOD_ASSIGN; }
	| _ADD_ASSIGN { $$ = ADD_ASSIGN; }
	| _SUB_ASSIGN { $$ = SUB_ASSIGN; }
	| _LEFT_ASSIGN { $$ = LEFT_ASSIGN; }
	| _RIGHT_ASSIGN { $$ = RIGHT_ASSIGN; }
	| _AND_ASSIGN { $$ = AND_ASSIGN; }
	| _XOR_ASSIGN { $$ = XOR_ASSIGN; }
	| _OR_ASSIGN { $$ = OR_ASSIGN; }

conditional_expression
	: logical_or_expression { $$ = $1; }
	| logical_or_expression '?' expression ':' conditional_expression { $$ = create_node_if(CONDITIONAL, $1, $3, $5); }

logical_or_expression
	: logical_and_expression { $$ = $1; }
	| logical_or_expression _OR_OP logical_and_expression { $$ = create_node2(LOR, $1, $3); }

logical_and_expression
	: inclusive_or_expression { $$ = $1; }
	| logical_and_expression _AND_OP inclusive_or_expression { $$ = create_node2(LAND, $1, $3); }

inclusive_or_expression
	: exclusive_or_expression { $$ = $1; }
	| inclusive_or_expression '|' exclusive_or_expression { $$ = create_node2(BOR, $1, $3); }

exclusive_or_expression
	: and_expression { $$ = $1; }
	| exclusive_or_expression '^' and_expression { $$ = create_node2(BXOR, $1, $3); }

and_expression
	: equality_expression { $$ = $1; }
	| and_expression '&' equality_expression { $$ = create_node2(BAND, $1, $3); }

equality_expression
	: relational_expression { $$ = $1; }
	| equality_expression _EQ_OP relational_expression { $$ = create_node2(EQ, $1, $3); }
	| equality_expression _NE_OP relational_expression { $$ = create_node2(NE, $1, $3); } 

relational_expression
	: shift_expression { $$ = $1; }
	| relational_expression '<' shift_expression { $$ = create_node2(LT, $1, $3); }
	| relational_expression '>' shift_expression { $$ = create_node2(GT, $1, $3); }
	| relational_expression _LE_OP shift_expression { $$ = create_node2(LE, $1, $3); }
	| relational_expression _GE_OP shift_expression { $$ = create_node2(GE, $1, $3); }

shift_expression
	: additive_expression { $$ = $1; }
	| shift_expression _LEFT_OP additive_expression  { $$ = create_node2(LEFT_SHIFT, $1, $3); }
	| shift_expression _RIGHT_OP additive_expression { $$ = create_node2(RIGHT_SHIFT, $1, $3); }

additive_expression
	: multiplicative_expression { $$ = $1; }
	| additive_expression '+' multiplicative_expression { $$ = create_node2(ADD, $1, $3); }
	| additive_expression '-' multiplicative_expression { $$ = create_node2(SUB, $1, $3); }

multiplicative_expression
	: cast_expression { $$ = $1; }
	| multiplicative_expression '*' cast_expression { $$ = create_node2(MUL, $1, $3); }
	| multiplicative_expression '/' cast_expression { $$ = create_node2(DIV, $1, $3); }
	| multiplicative_expression '%' cast_expression { $$ = create_node2(REM, $1, $3); }

cast_expression
	: unary_expression { $$ = $1; }
	| '(' type_name ')' cast_expression { ($$ = create_node1(CAST, $4))->ts = $2; }

type_name
	: specifier_qualifier_list { $$ = $1; }
	/* | specifier_qualifier_list abstract_declarator */

specifier_qualifier_list
	: type_specifier specifier_qualifier_list { $$ = (enum type)($1 | $2); }
	| type_specifier { $$ = (enum type) $1; }
	| type_qualifier specifier_qualifier_list { $$ = (enum type)($1 | $2); }
	| type_qualifier { $$ = (enum type) $1; }

unary_expression
	: postfix_expression { $$ = $1; }
	| _INC_OP unary_expression { $$ = create_node1(PRE_INC, $2); }
	| _DEC_OP unary_expression { $$ = create_node1(PRE_DEC, $2); }
	| unary_operator cast_expression { $$ = create_node1($1, $2); }
	/* | SIZEOF unary_expression */
	/* | SIZEOF '(' type_name ')' */

unary_operator
	: '&' { $$ = REFOF; }
	| '*' { $$ = DEREF; }
	| '+' { $$ = UPLUS; }
	| '-' { $$ = UMINUS; }
	| '~' { $$ = BNOT; }
	| '!' { $$ = LOGICAL_NOT; }

postfix_expression
	: primary_expression { $$ = $1; }
	/* | postfix_expression '[' expression ']' */
	/* | postfix_expression '(' ')' */
	/* | postfix_expression '(' argument_expression_list ')' */
	/* | postfix_expression '.' IDENTIFIER */
	/* | postfix_expression PTR_OP IDENTIFIER */
	| postfix_expression _INC_OP { $$ = create_node1(POST_INC, $1); }
	| postfix_expression _DEC_OP { $$ = create_node1(POST_DEC, $1); }

primary_expression
    : _IDENTIFIER                                     { $$ = create_node_literal(ID, -1, $1); }
    | _NUM                                            { $$ = create_node_literal(NUM, INT /* int32 by default, overriden by declaration type */, (void*)(intptr_t)$1); }
	/* | CONSTANT instead of NUM ...? */
	/* | STRING_LITERAL */
	| '(' expression ')' { $$ = $2; }

jump_statement
	/* : GOTO IDENTIFIER ';' */
	/* | CONTINUE ';' */
	/* | BREAK ';' */
	: _RETURN ';'             { $$ = create_node1(RETURN, NULL); }
	| _RETURN expression ';'  { $$ = create_node1(RETURN, $2); }

/* iteration_statement */
/* 	| WHILE '(' expression ')' statement */
/* 	: DO statement WHILE '(' expression ')' ';' */
/* 	| FOR '(' expression_statement expression_statement ')' statement */
/* 	| FOR '(' expression_statement expression_statement expression ')' statement */

selection_statement
	: _IF '(' expression ')' statement { $$ = create_node_if(IF, $3, $5, NULL); }
	| _IF '(' expression ')' statement _ELSE statement { $$ = create_node_if(IF, $3, $5, $7); }
	/* | SWITCH '(' expression ')' statement */

%%

void yyerror(const char* str) {
    fprintf(stderr,"Syntax error: %s\n", str);
    fprintf(stderr,"Exiting...\n");
    exit(2);
}

