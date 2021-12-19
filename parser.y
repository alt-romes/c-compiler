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
    struct declaration declaration_v;
    enum type declaration_specifiers_v;
}

%token _INT _SHORT _CHAR _UNSIGNED _SIGNED
%token _CONST
%token _NUM
%token _IDENTIFIER
%token _EQ_OP _NE_OP _LE_OP _GE_OP _OR_OP _AND_OP _LEFT_OP _RIGHT_OP

%type <int_v> _NUM type_specifier type_qualifier
%type <string_v> _IDENTIFIER declarator direct_declarator // string is malloc'd and needs to be freed by the ast destructor
%type <node_v> initializer statement_list compound_statement function_definition expression assignment_expression conditional_expression logical_or_expression logical_and_expression inclusive_or_expression exclusive_or_expression and_expression equality_expression relational_expression shift_expression additive_expression multiplicative_expression cast_expression unary_expression postfix_expression primary_expression statement expression_statement
%type <declaration_list_v> declaration_list declaration init_declarator_list
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
    : '{' statement_list '}'                          { $$ = $2; }
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
	: statement { $$ = $1; }
	| statement_list statement /* TODO: How are sequential statements different from a sequence of expressions. They're currently the same to make it easier but ... the types are wrong, etc... */ { $$ = create_node2(SEQEXP, $1, $2); }

statement
	/* : labeled_statement */
	: compound_statement { $$ = $1; }
	| expression_statement { $$ = $1; }
	/* | selection_statement */
	/* | iteration_statement */
	/* | jump_statement */

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
    /* | MUL_ASSIGN */
	/* | DIV_ASSIGN */
	/* | MOD_ASSIGN */
	/* | ADD_ASSIGN */
	/* | SUB_ASSIGN */
	/* | LEFT_ASSIGN */
	/* | RIGHT_ASSIGN */
	/* | AND_ASSIGN */
	/* | XOR_ASSIGN */
	/* | OR_ASSIGN */

conditional_expression
	: logical_or_expression { $$ = $1; }
	/* | logical_or_expression '?' expression ':' conditional_expression */

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
	/* | '(' type_name ')' cast_expression */

unary_expression
	: postfix_expression { $$ = $1; }
	/* | INC_OP unary_expression */
	/* | DEC_OP unary_expression */
	| unary_operator cast_expression { $$ = create_node1($1, $2); }
	/* | SIZEOF unary_expression */
	/* | SIZEOF '(' type_name ')' */

unary_operator
	/* : '&' */
	/* | '*' */
	: '+' { $$ = UPLUS; }
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
	/* | postfix_expression INC_OP */
	/* | postfix_expression DEC_OP */

primary_expression
    : _IDENTIFIER                                     { $$ = create_node_literal(ID, -1, $1); }
    | _NUM                                            { $$ = create_node_literal(NUM, INT /* int32 by default, overriden by declaration type */, (void*)(intptr_t)$1); }
	/* | CONSTANT instead of NUM ...? */
	/* | STRING_LITERAL */
	| '(' expression ')' { $$ = $2; }

%%

void yyerror(const char* str) {
    fprintf(stderr,"Syntax error: %s\n", str);
    fprintf(stderr,"Exiting...\n");
    exit(2);
}

