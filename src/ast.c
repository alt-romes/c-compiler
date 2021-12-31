#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <environment.h>
#include <ast.h>

node_t* new_node(node_type_t type) {

    node_t* node;

    switch (type) {
        case FUNCTION:
            node = malloc(sizeof(function_node_t));
            break;
        case BLOCK:
            node = malloc(sizeof(block_node_t));
            break;
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case REM:
        case LT:
        case GT:
        case LE:
        case GE:
        case EQ:
        case NE:
        case LOR:
        case LAND:
        case BOR:
        case BXOR:
        case BAND:
        case LEFT_SHIFT:
        case RIGHT_SHIFT:
        case SEQEXP:
        case ASSIGN:
        case MUL_ASSIGN:
        case DIV_ASSIGN:
        case MOD_ASSIGN:
        case ADD_ASSIGN:
        case SUB_ASSIGN:
        case LEFT_ASSIGN:
        case RIGHT_ASSIGN:
        case AND_ASSIGN:
        case XOR_ASSIGN:
        case OR_ASSIGN:
            node = malloc(sizeof(binary_node_t));
            break;
        case LOGICAL_NOT:
        case UMINUS:
        case UPLUS:
        case BNOT:
        case RETURN:
        case CAST:
        case PRE_INC:
        case PRE_DEC:
        case POST_INC:
        case POST_DEC:
        case REFOF:
        case DEREF:
            node = malloc(sizeof(unary_node_t));
            break;
        case ID:
            node = malloc(sizeof(id_node_t));
            break;
        case NUM:
        case UNIT:
            node = malloc(sizeof(node_t));
            break;
        case IF:
        case CONDITIONAL:
            node = malloc(sizeof(if_node_t));
            break;
    }

    node->type = type;
    node->ts = type_from(UNDEFINED);

    return node;
}

node_t* create_node_literal(node_type_t type, type_t ts, void* literal_value) {
    
    node_t* node = new_node(type);
    node->ts = ts;
    
    switch (type) {
        case NUM: {
            ((num_node_t*)node)->value = (int)(intptr_t)literal_value;
            break;
        }
        case ID:
            ((id_node_t*)node)->value = (char*)literal_value;
            break;
        case UNIT:
            break;
        default:
            fprintf(stderr, "ERROR: Literal node should have type NUM, BOOL or ID!\n");
            exit(1);
    }

    return node;
}

node_t* create_node1(node_type_t type, node_t* n) {

    unary_node_t* node = (unary_node_t*)new_node(type);
    node->child = n;

    /* Desugaring */
    switch (type) {
        case PRE_INC:
            node->child = create_node2(ADD_ASSIGN, node->child, create_node_literal(NUM, node->ts, (void*)(intptr_t)1));
            break;
        case PRE_DEC:
            node->child = create_node2(SUB_ASSIGN, node->child, create_node_literal(NUM, node->ts, (void*)(intptr_t)1));
            break;
        case POST_INC:
            node->child = create_node2(SUB, create_node2(ADD_ASSIGN, node->child, create_node_literal(NUM, node->ts, (void*)(intptr_t)1)), create_node_literal(NUM, node->ts, (void*)(intptr_t)1));
            break;
        case POST_DEC:
            node->child = create_node2(ADD, create_node2(SUB_ASSIGN, node->child, create_node_literal(NUM, node->ts, (void*)(intptr_t)1)), create_node_literal(NUM, node->ts, (void*)(intptr_t)1));
            break;
        
        default: break;
    }

    return (node_t*)node;
}

node_t* create_node2(node_type_t type, node_t* l, node_t* r) {
     
    binary_node_t* node = (binary_node_t*)new_node(type);
    node->left = l;
    node->right = r;

    /* Desugaring */
    node_type_t aux = 0;
    switch (type) {
        case MUL_ASSIGN: aux = MUL; break;
        case DIV_ASSIGN: aux = DIV; break;
        case MOD_ASSIGN: aux = REM; break;
        case ADD_ASSIGN: aux = ADD; break;
        case SUB_ASSIGN: aux = SUB; break;
        case LEFT_ASSIGN: aux = LEFT_SHIFT; break;
        case RIGHT_ASSIGN: aux = RIGHT_SHIFT; break;
        case AND_ASSIGN: aux = BAND; break;
        case XOR_ASSIGN: aux = BXOR; break;
        case OR_ASSIGN: aux = BOR; break;
        default: break;
    }

    switch (type) {
        case MUL_ASSIGN: case DIV_ASSIGN: case MOD_ASSIGN:
        case ADD_ASSIGN: case SUB_ASSIGN:
        case LEFT_ASSIGN: case RIGHT_ASSIGN:
        case AND_ASSIGN: case XOR_ASSIGN: case OR_ASSIGN:
            // Desugar XXX_ASSIGN to normal assign, but keep type to correctly free memory.
            // These types are to be treated exactly as an ASSIGN because that's what they really are under the hood
            ((binary_node_t*)node)->right =
                create_node2(aux, ((binary_node_t*)node)->left, ((binary_node_t*)node)->right);
            break;
        default: break;
    }

    return (node_t*)node;
}

node_t* create_node_block(node_type_t type, declaration_list_t* declaration_list, statement_list_t* statement_list) {

    block_node_t* node = (block_node_t*)new_node(type);
    node->declaration_list = declaration_list;
    node->statement_list = statement_list;
    return (node_t*)node;
}

node_t* create_node_function(node_type_t type, struct declarator decl, node_t* b) {

    function_node_t* node = (function_node_t*)new_node(type);
    node->decl = decl;
    node->body = b;
    return (node_t*)node;
}

node_t* create_node_if(node_type_t type, node_t* cond, node_t* thenst, node_t* elsest) {

    if_node_t* node = (if_node_t*)new_node(type);
    node->cond = cond;
    node->thenst = thenst;
    node->elsest = elsest;
    return (node_t*)node;
}

void free_ast(node_t* node) {

    switch (node->type) {
        case FUNCTION: {
            // Free declaration id and args list
            free(((function_node_t*)node)->decl.id);

            /* if (((function_node_t*)node)->decl.args != NULL && ((function_node_t*)node)->decl.args->args != NULL) */
            /*     free(((function_node_t*)node)->decl.args->args); */
            /* free(((function_node_t*)node)->decl.args); */

            free_ast(((function_node_t*)node)->body);
            break;
        }
        case BLOCK: {
            declaration_list_t* dae = ((block_node_t*)node)->declaration_list;
            statement_list_t* stmtl = ((block_node_t*)node)->statement_list;
            for (int i = 0; i < dae->size; i++) {
                free(dae->declarations[i].id);
                if (dae->declarations[i].node != NULL)
                    free_ast(dae->declarations[i].node);
            }

            free(dae->declarations);
            free(dae);

            // Free all nodes in statements list
            for (int i = 0; i < stmtl->size; i++)
                free_ast(stmtl->statements[i]);
            // Free statements malloc'd array
            free(stmtl->statements);
            // Free statement list malloc'd structure
            free(stmtl);

            break;
        }
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case REM:
        case LT:
        case GT:
        case LE:
        case GE:
        case EQ:
        case NE:
        case LOR:
        case LAND:
        case BOR:
        case BXOR:
        case BAND:
        case LEFT_SHIFT:
        case RIGHT_SHIFT:
        case SEQEXP:
        case ASSIGN:
            free_ast(((binary_node_t*)node)->left);
            free_ast(((binary_node_t*)node)->right);
            break;
        case MUL_ASSIGN:
        case DIV_ASSIGN:
        case MOD_ASSIGN:
        case ADD_ASSIGN:
        case SUB_ASSIGN:
        case LEFT_ASSIGN:
        case RIGHT_ASSIGN:
        case AND_ASSIGN:
        case XOR_ASSIGN:
        case OR_ASSIGN:
            // Don't free left ast because the XXX_ASSIGN node has been desugered
            // from x *= y to x = x * y, where a pointer to x is on both the left and the right hand side.
            // As such, the left hand side will be automatically freed when teh right hand side is freed.
            free_ast(((binary_node_t*)node)->right);
            break;
        case LOGICAL_NOT:
        case UMINUS:
        case UPLUS:
        case BNOT:
        case RETURN:
        case CAST:
        case PRE_INC:
        case PRE_DEC:
        case POST_INC:
        case POST_DEC:
        case REFOF:
        case DEREF:
            if (((unary_node_t*)node)->child != NULL)
                free_ast(((unary_node_t*)node)->child);
            break;
        case ID:
            free(((id_node_t*)node)->value);
            break;
        case NUM:
        case UNIT:
            break;
        case IF:
        case CONDITIONAL:
            free_ast(((if_node_t*)node)->cond);
            free_ast(((if_node_t*)node)->thenst);
            if (((if_node_t*)node)->elsest != NULL)
                free_ast(((if_node_t*)node)->elsest);
            break;
    }

    free(node);
}


/* Declaration List */
declaration_list_t* create_declaration_list() {
    declaration_list_t* dl = malloc(sizeof(declaration_list_t));
    dl->declarations = NULL; // Important because realloc requires a NULL pointer to behave as malloc
    dl->size = 0;            // Important to set because when attempting to associate a value if it's an undefined value things can go wrong
    return dl;
}

declaration_list_t* declaration_list_assoc(declaration_list_t* e, struct declaration d) {

    if (!e->size % DEFAULT_ENVIRONMENT_SIZE)
        // CAREFUL: e->declarations must be initially NULL for realloc to behave as malloc!
        e->declarations = realloc(e->declarations, (e->size+DEFAULT_ENVIRONMENT_SIZE)*sizeof(struct declaration));

    e->declarations[e->size++] = d;

    return e;
}

declaration_list_t* declaration_list_merge(declaration_list_t* src, declaration_list_t* dst) {

    for (int i = 0; i < src->size; i++)
        declaration_list_assoc(dst, src->declarations[i]);

    free(src->declarations);
    free(src);
    
    return dst;
}

declaration_list_t* add_declaration_specifiers(declaration_list_t* decs, type_t et) {
    
    for (struct declaration* d = decs->declarations, * lim = d + decs->size; d < lim; d++) d->et = set_base_type(d->et, et);

    return decs;
}


/* Statement List */
statement_list_t* create_statement_list() {
    statement_list_t* l = malloc(sizeof(statement_list_t));
    l->statements = NULL;   // Important because realloc requires a NULL pointer to behave as malloc
    l->size = 0;            // Important to set because when attempting to associate a value if it's an undefined value things can go wrong
    return l;
}

statement_list_t* statement_list_add(statement_list_t * e, node_t* node) {

    if (!e->size % DEFAULT_ENVIRONMENT_SIZE)
        // CAREFUL: e->declarations must be initially NULL for realloc to behave as malloc!
        e->statements = realloc(e->statements, (e->size+DEFAULT_ENVIRONMENT_SIZE)*sizeof(node_t*));

    e->statements[e->size++] = node;

    return e;
}


struct args_list* create_args_list() {
    struct args_list* l = malloc(sizeof(struct args_list));
    l->args = NULL;   // Important because realloc requires a NULL pointer to behave as malloc
    l->size = 0;            // Important to set because when attempting to associate a value if it's an undefined value things can go wrong
    return l;
}

struct args_list* args_list_add(struct args_list* e, struct declarator v) {

    if (!e->size % DEFAULT_ENVIRONMENT_SIZE)
        e->args = realloc(e->args, (e->size+DEFAULT_ENVIRONMENT_SIZE)*sizeof(struct declarator));

    e->args[e->size++] = v;

    return e;
}
