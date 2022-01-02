#ifndef _AST_H
#define _AST_H

#include "environment.h"
#include "types.h"

struct args_list {
    struct declarator* args;
    int size;
};

struct args_list* create_args_list();
struct args_list* args_list_add(struct args_list*, struct declarator);

struct declarator {
    char* id;
    type_t ts;
};

/* Declaration List */

struct declaration {
    char* id;
    type_t et;
    struct node* node;
};

typedef struct declaration_list {
    struct declaration* declarations;
    int size;
} declaration_list_t; 

declaration_list_t* create_declaration_list();
declaration_list_t* declaration_list_assoc(declaration_list_t* e, struct declaration d);
declaration_list_t* declaration_list_merge(declaration_list_t* src, declaration_list_t* dst); // merge two environments by copying all associations from src to dst, freeing src, and keeping dst's parent
declaration_list_t* add_declaration_specifiers(declaration_list_t*, type_t);
declaration_list_t* extend_declaration_specifiers(declaration_list_t*, enum type);

/* Statement List */

typedef struct statement_list {
    struct node** statements;
    int size;
} statement_list_t; 

statement_list_t* create_statement_list();
statement_list_t* statement_list_add(statement_list_t*, struct node*);

/*
 * All `*_node` structures must start with a node_type_t field so that they can
 * be cast from and to `struct node`
 */

typedef enum node_type {
    NUM,
    ID,

    /* Binary */
    ADD,
    SUB,
    MUL,
    DIV,
    REM,
    EQ,
    NE,
    LT,
    GT,
    LE,
    GE,
    LOR,
    LAND,
    BAND,
    BOR,
    BXOR,
    LEFT_SHIFT,
    RIGHT_SHIFT,
    SEQEXP,
    ASSIGN,
    MUL_ASSIGN,
    DIV_ASSIGN,
    MOD_ASSIGN,
    ADD_ASSIGN,
    SUB_ASSIGN,
    LEFT_ASSIGN,
    RIGHT_ASSIGN,
    AND_ASSIGN,
    XOR_ASSIGN,
    OR_ASSIGN,

    /* Unary */
    PRE_INC,
    PRE_DEC,
    POST_INC,
    POST_DEC,
    REFOF,
    DEREF,
    UPLUS,
    UMINUS,
    LOGICAL_NOT,
    BNOT,
    RETURN,

    IF,
    CONDITIONAL,

    CAST,
    BLOCK,
    GLOBAL_BLOCK,
    FUNCTION,
    UNIT
} node_type_t;

typedef struct node {
    node_type_t type;
    type_t ts;
} node_t;

typedef struct num_node {
    node_type_t type;
    type_t ts;
    int value;
} num_node_t;

typedef struct id_node {
    node_type_t type;
    type_t ts;
    char* value;
} id_node_t;

typedef struct unary_node {
    node_type_t type;
    type_t ts;
    struct node* child;
} unary_node_t;

typedef struct binary_node {
    node_type_t type;
    type_t ts;
    struct node* left;
    struct node* right;
} binary_node_t;

typedef struct block_node {
    node_type_t type;
    type_t ts;
    declaration_list_t* declaration_list;
    statement_list_t* statement_list;
} block_node_t;

typedef struct function_node {
    node_type_t type;
    type_t ts;
    struct declarator decl;
    struct node* body;
} function_node_t;

typedef struct if_node {
    node_type_t type;
    type_t ts;
    struct node* cond;
    struct node* thenst;
    struct node* elsest;
} if_node_t;

node_t* create_node_num(node_type_t, type_t, int);
node_t* create_node_id(node_type_t, type_t, char*);
node_t* create_node_unit(node_type_t, type_t);
node_t* create_node1(node_type_t, node_t*);
node_t* create_node2(node_type_t, node_t*, node_t*);
node_t* create_node_block(node_type_t, declaration_list_t*, statement_list_t*);
node_t* create_node_function(node_type_t, struct declarator, node_t*);
node_t* create_node_if(node_type_t, node_t*, node_t*, node_t*);

void free_ast(node_t* root);

#endif
