%{

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "ast.h"

int yylex(); // defined by lex
void yyerror();

void fail(char* s) {

    fprintf(stderr, "[Failed] %s\n", s);
    exit(-2);
}

%}

%union {
    int int_v;
    char* string_v;
    struct node* node_v;
    node_type_t node_type_v;
    struct declaration_list* declaration_list_v;
    struct statement_list* statement_list_v;
    struct declaration declaration_v;
    struct type_s* struct_type_v;
    enum type enum_type_v;
    struct declarator declarator_v;
    struct args_list* args_list_v;
}

%token _VOID _CHAR _SHORT _INT _LONG _UNSIGNED _SIGNED
%token _CONST
%token _NUM
%token _IDENTIFIER
%token _EQ_OP _NE_OP _LE_OP _GE_OP _OR_OP _AND_OP _LEFT_OP _RIGHT_OP
%token _MUL_ASSIGN _DIV_ASSIGN _MOD_ASSIGN _ADD_ASSIGN _SUB_ASSIGN _LEFT_ASSIGN _RIGHT_ASSIGN _AND_ASSIGN _XOR_ASSIGN _OR_ASSIGN _INC_OP _DEC_OP
%token _RETURN _IF _ELSE

%type <int_v> _NUM
%type <string_v> _IDENTIFIER // string is malloc'd and needs to be freed by the ast destructor
%type <declarator_v> declarator direct_declarator parameter_declaration
%type <node_v> initializer compound_statement function_definition expression assignment_expression conditional_expression logical_or_expression logical_and_expression inclusive_or_expression exclusive_or_expression and_expression equality_expression relational_expression shift_expression additive_expression multiplicative_expression cast_expression unary_expression postfix_expression primary_expression statement expression_statement jump_statement selection_statement constant_expression
%type <declaration_list_v> declaration_list declaration init_declarator_list
%type <statement_list_v> statement_list
%type <declaration_v> init_declarator
%type <enum_type_v> declaration_specifiers specifier_qualifier_list type_specifier type_qualifier type_qualifier_list
%type <struct_type_v> pointer abstract_declarator type_name
%type <node_type_v> unary_operator assignment_operator
%type <args_list_v> parameter_type_list parameter_list

%parse-param {node_t** root}

%start init

%%

init
    : function_definition                             { *root = $1; }

/* translation_unit */
/* 	: external_declaration */
	/* | translation_unit external_declaration */

/* external_declaration */
/* 	: function_definition */
	/* | declaration */

function_definition
    : declaration_specifiers declarator compound_statement { if ($2.ts->t == UNDEFINED || !($2.ts->t & FUNCTION_TYPE)) fail("Function declarator does not define a function!");
                                                             $2.ts = set_base_type($2.ts, type_from($1));
                                                             $$ = create_node_function(FUNCTION, $2, $3); }
	| declarator compound_statement { $1.ts = set_base_type($1.ts, type_from(INT)); $$ = create_node_function(FUNCTION, $1, $2); }

compound_statement
    : '{' '}'                                         { $$ = create_node_literal(UNIT, type_from(VOID), NULL); }
    | '{' statement_list '}'                          { $$ = create_node_block(BLOCK, create_declaration_list() /* empty by default */ , $2); }
    | '{' declaration_list '}'                        { $$ = create_node_block(BLOCK, $2, create_statement_list() /* empty by default */); }
    | '{' declaration_list statement_list '}'         { $$ = create_node_block(BLOCK, $2, $3); }

declaration_list
    : declaration                                     { $$ = $1; }
    | declaration_list declaration                    { $$ = declaration_list_merge($2, $1); }

declaration
    : declaration_specifiers ';'                      { $$ = create_declaration_list(); /* does nothing */ }
    | declaration_specifiers init_declarator_list ';' { $$ = add_declaration_specifiers($2, type_from($1)); }

declaration_specifiers
    /* : storage_class_specifier */
    /* | storage_class_specifier declaration_specifiers */
    : type_specifier                                  { $$ = (enum type) $1; }
    | type_specifier declaration_specifiers           { $$ = (enum type)($1 | $2); }
    | type_qualifier                                  { $$ = (enum type) $1; } // declarations can end in e.g. "const"
    | type_qualifier declaration_specifiers           { $$ = (enum type)($1 | $2); }

/* storage_class_specifier */
/* 	: TYPEDEF */
/* 	| EXTERN */
/* 	| STATIC */
/* 	| AUTO */
/* 	| REGISTER */

type_qualifier
    : _CONST                                          { $$ = CONST; }
    /* | VOLATILE */

type_specifier
    : _VOID                                           { $$ = VOID;      }
    | _CHAR                                           { $$ = CHAR;      }
    | _SHORT                                          { $$ = SHORT;     }
    | _INT                                            { $$ = INT;       }
	| _LONG                                           { $$ = LONG;      }
	/* | _FLOAT */
	/* | _DOUBLE */
    | _SIGNED                                         { $$ = SIGNED;    }
    | _UNSIGNED                                       { $$ = UNSIGNED;  }
	/* | struct_or_union_specifier */
	/* | enum_specifier */
	/* | _TYPE_NAME */

/* struct_or_union_specifier */
/* 	: struct_or_union _IDENTIFIER '{' struct_declaration_list '}' */
/* 	| struct_or_union '{' struct_declaration_list '}' */
/* 	| struct_or_union _IDENTIFIER */

/* struct_or_union */
/* 	: _STRUCT */
/* 	| _UNION */

/* struct_declaration_list */
/* 	: struct_declaration */
/* 	| struct_declaration_list struct_declaration */

/* struct_declaration */
/* 	: specifier_qualifier_list struct_declarator_list ';' */

/* struct_declarator_list */
/* 	: struct_declarator */
/* 	| struct_declarator_list ',' struct_declarator */

/* struct_declarator */
/* 	: declarator */
/* 	| ':' constant_expression */
/* 	| declarator ':' constant_expression */

/* enum_specifier */
/* 	: ENUM '{' enumerator_list '}' */
/* 	| ENUM IDENTIFIER '{' enumerator_list '}' */
/* 	| ENUM IDENTIFIER */

/* enumerator_list */
/* 	: enumerator */
/* 	| enumerator_list ',' enumerator */

/* enumerator */
/* 	: IDENTIFIER */
/* 	| IDENTIFIER '=' constant_expression */

init_declarator_list
    : init_declarator                                 { $$ = declaration_list_assoc(create_declaration_list(), $1); }
    | init_declarator_list ',' init_declarator        { $$ = declaration_list_assoc($1, $3); }

init_declarator
    : declarator                                      { $$ = (struct declaration){ $1.id, $1.ts, .node = NULL }; }
    | declarator '=' initializer                      { $$ = (struct declaration){ $1.id, $1.ts, .node = $3 }; }

declarator
	: pointer direct_declarator                       { $2.ts = set_base_type($2.ts, $1); $$ = $2; }
	| direct_declarator                               { $$ = $1; }

pointer
	: '*' { $$ = create_type_pointer(POINTER); }
	| '*' type_qualifier_list { $$ = create_type_pointer(POINTER | $2); }
	| '*' pointer { $$ = set_base_type($2, create_type_pointer(POINTER)); }
	| '*' type_qualifier_list pointer { $$ = set_base_type($3, create_type_pointer(POINTER | $2)); }

type_qualifier_list
	: type_qualifier { $$ = $1; }
	| type_qualifier_list type_qualifier { $$ = (enum type)($1 | $2); }

direct_declarator
    : _IDENTIFIER                                     { $$ = (struct declarator){ .id = $1, .ts = type_from(UNDEFINED), .args = NULL }; }
    | '(' declarator ')'                              { $$ = $2; }
	/* | direct_declarator '[' constant_expression ']' */
	/* | direct_declarator '[' ']'                       { $$ = (struct declarator){ .id = $1.id, .ts = REFERENCE + $1.ts, .args = NULL }; } */
	| direct_declarator '(' parameter_type_list ')'   { $1.ts = set_base_type($1.ts, create_type_function(FUNCTION_TYPE, $3));
                                                        $1.args = $1.args == NULL ? $3 : $1.args; // inner-most declator is the one defining the function name and arguments
                                                        $$ = $1; }
	/* | direct_declarator '(' identifier_list ')' */
	| direct_declarator '(' ')' { { struct args_list* args_list = create_args_list();
                                    $1.ts = set_base_type($1.ts, create_type_function(FUNCTION_TYPE, args_list));
                                    $1.args = $1.args == NULL ? args_list : $1.args;
                                    $$ = $1;
                                } }

parameter_type_list
	: parameter_list { $$ = $1; }
	/* | parameter_list ',' ELLIPSIS */

parameter_list
	: parameter_declaration { $$ = args_list_add(create_args_list(), $1); }
	| parameter_list ',' parameter_declaration { $$ = args_list_add($1, $3); }

parameter_declaration
	: declaration_specifiers declarator { $2.ts = set_base_type($2.ts, type_from($1)); $$ = $2; }
	/* | declaration_specifiers abstract_declarator */
	/* | declaration_specifiers */

/* identifier_list */
/* 	: IDENTIFIER */
/* 	| identifier_list ',' IDENTIFIER */

initializer
    : assignment_expression                                             { $$ = $1; }
    /* | '{' initializer_list '}' */
    /* | '{' initializer_list ',' '}' */

/* initializer_list */
/* 	: initializer */
/* 	| initializer_list ',' initializer */

statement_list
	: statement { $$ = statement_list_add(create_statement_list(), $1); }
	| statement_list statement { $$ = statement_list_add($1, $2); }

statement
	/* : labeled_statement */
	: compound_statement { $$ = $1; }
	| expression_statement { $$ = $1; }
	| selection_statement { $$ = $1; }
	/* | iteration_statement */
	| jump_statement { $$ = $1; }

/* labeled_statement */
/* 	: _IDENTIFIER ':' statement */
/* 	| _CASE constant_expression ':' statement */
/* 	| _DEFAULT ':' statement */

expression_statement
	: ';'            { $$ = create_node_literal(UNIT, type_from(VOID), NULL); }
	| expression ';' { $$ = $1; }

constant_expression
	: conditional_expression { $$ = $1; }

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
	: specifier_qualifier_list { $$ = type_from($1); }
	| specifier_qualifier_list abstract_declarator { $$ = set_base_type($2, type_from($1)); }

specifier_qualifier_list
	: type_specifier specifier_qualifier_list { $$ = (enum type)($1 | $2); }
	| type_specifier { $$ = (enum type) $1; }
	| type_qualifier specifier_qualifier_list { $$ = (enum type)($1 | $2); }
	| type_qualifier { $$ = (enum type) $1; }

abstract_declarator
	: pointer { $$ = $1; }
	/* | direct_abstract_declarator */
	/* | pointer direct_abstract_declarator */

/* direct_abstract_declarator */
/* 	: '(' abstract_declarator ')' */
/* 	| '[' ']' */
/* 	| '[' constant_expression ']' */
/* 	| direct_abstract_declarator '[' ']' */
/* 	| direct_abstract_declarator '[' constant_expression ']' */
/* 	| '(' ')' */
/* 	| '(' parameter_type_list ')' */
/* 	| direct_abstract_declarator '(' ')' */
/* 	| direct_abstract_declarator '(' parameter_type_list ')' */


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

/* argument_expression_list */
/* 	: assignment_expression */
/* 	| argument_expression_list ',' assignment_expression */

primary_expression
    : _IDENTIFIER                                     { $$ = create_node_literal(ID, type_from(UNDEFINED), $1); }
    | _NUM                                            { $$ = create_node_literal(NUM, type_from(INT) /* int32 by default, overriden by declaration type */, (void*)(intptr_t)$1); }
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


/*

NO SUPPORT FOR, FROM THE ANSI C GRAMMAR:

function_definition
--> : declaration_specifiers declarator declaration_list compound_statement
    | declaration_specifiers declarator compound_statement { $$ = create_node_function(FUNCTION, $1, $2, $3); }
--> | declarator declaration_list compound_statement
    | declarator compound_statement { $$ = create_node_function(FUNCTION, INT, $1, $2); }

*/
