#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "typecheck.h"

type_t typecheck(struct node* node, struct environment* e) {
    type_t t;
    switch (node->type) {
        case FUNCTION: {
            // TODO ... 
            /* this doesn't need to be, only make sure that its castable from one to another assert((t = node->ts) == typecheck(((function_node_t*)node)->body, e)); */
            t = node->ts; // function type is its return value... quite wrong but ..

            environment_t* scope_env = beginScope(e);

            /* if (((function_node_t*)node)->decl.args != NULL) // If function has parameters (same as above) */
            /*     // Add params types to environment */
            /*     for (int i = 0; i < ((function_node_t*)node)->decl.args->size; i++) */
            /*         assoc(scope_env, ((function_node_t*)node)->decl.args->args[i].id, (union association_v){ .type = ((function_node_t*)node)->decl.args->args[i].ts }); */


            typecheck(((function_node_t*)node)->body, scope_env);


            endScope(scope_env); // free the scope environment and its association array

            break;
        }
        case ID:
            // Set this node's type to the one found in the environment
            t = find(e, ((id_node_t*)node)->value).type;
            break;
        case BLOCK: {

            environment_t* scope_env = beginScope(e);

            block_node_t* bnode = (block_node_t*)node;
            declaration_list_t* dae = bnode->declaration_list;  // this id->ast_node environment is freed when the whole ast is freed
                                                                // NOTE: the ids will be freed with the ast (they were allocated with it, so they should be deallocated with it)

            // For each declaration in this scope create an association in this scope's evaluation environment
            for (int i = 0; i < dae->size; i++) {
                /* dae->declarations[i].node->ts = dae->declarations[i].et; this isn't needed, a cast should be made when associating a value to an identifier */
                if (dae->declarations[i].node != NULL)
                    typecheck(dae->declarations[i].node, scope_env); // Must still typecheck all declaration values
                /* assert(tychkty == dae->declarations[i].node->ts); No longer true, the value doesn't need to have the same time because it'll be casted on assignment */
                assoc(scope_env, dae->declarations[i].id, (union association_v){ .type = dae->declarations[i].et });
            }

            statement_list_t* stmtl = ((block_node_t*)node)->statement_list;
            for (int i = 0; i < stmtl->size; i++)
                // Typecheck each statement
                typecheck(stmtl->statements[i], scope_env);
            /* t = typecheck(((block_node_t*)node)->body, scope_env); */

            endScope(scope_env); // free the scope environment and its association array

            t = type_from(VOID);

            break;
        }
        case NUM:
            // TODO: Just Min required value to hold this number, ignore node->ts
            /* t = node->ts == 0 ? INT : node->ts; // Use already assigned type for num (on declaration) if available */
            t = type_from(INT);
            break;

        // All these operations are applied on numeric types that nust 
        case BOR:
        case BXOR:
        case BAND:
        case LEFT_SHIFT:
        case ADD:
        case SUB:
        case MUL: {
            type_t l = typecheck(((binary_node_t*)node)->left, e);
            type_t r = typecheck(((binary_node_t*)node)->right, e);
            // TODO: assert both types are numeric?
            t = type_compare(l, r) < 0 ? r : l;
            break;
        }
        case REM:
        case DIV: {
            type_t l = typecheck(((binary_node_t*)node)->left, e);
            type_t r = typecheck(((binary_node_t*)node)->right, e);
            assert(is_type_unsigned(l) == is_type_unsigned(r)); // For division either both values or unsigned or none is
            t = type_compare(l, r) < 0 ? r : l;
            break;
        }

        case RIGHT_SHIFT: {
            type_t l = typecheck(((binary_node_t*)node)->left, e);
            type_t r = typecheck(((binary_node_t*)node)->right, e);
            // For right shift do sign-extended if lhs is signed, zero fill otherwise
            t = extend_base_type((type_compare(l, r) < 0 ? r : l), (is_type_unsigned(l) ? UNSIGNED : SIGNED));
            break;
        }

        case SEQEXP: {
            typecheck(((binary_node_t*)node)->left, e);
            t = typecheck(((binary_node_t*)node)->right, e);
            break;
        }
        
        
        // All XXX_ASSIGNS have been desugered to an ASSIGN,
        // but maintain their type to be correctly freed.
        // Treat them equally.
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
        case OR_ASSIGN: {
            t = typecheck(((binary_node_t*)node)->left, e);
            if (is_type_const(t)) puts("Cannot assign to a variable qualified as `const`!");
            assert(!is_type_const(t)); // What else can i assert here? they both have the same time but can be cast i guess
            t = typecheck(((binary_node_t*)node)->right, e);
            break;
        }

        case LT:
        case GT:
        case LE:
        case GE: {
            type_t l = typecheck(((binary_node_t*)node)->left, e);
            type_t r = typecheck(((binary_node_t*)node)->right, e);
            assert(is_type_unsigned(l) == is_type_unsigned(r)); // Both types must be the same
            t = type_from(I1 | (is_type_unsigned(l) ? UNSIGNED : SIGNED)); // Return boolean encoded in char. If values are unsigned do unsigned comparison
            break;
        }
        case EQ:
        case NE:
            /* enum type l = */ typecheck(((binary_node_t*)node)->left, e);
            /* enum type r = */ typecheck(((binary_node_t*)node)->right, e);
            // Correction to the below: types can be ints with different size that will be promoted
            /* assert(type_compare(l, r) == 0); // Both types must be the same */
            t = type_from(I1); // I1 boolean
            break;
        case LOR:
        case LAND:
            /* enum type l = */ typecheck(((binary_node_t*)node)->left, e);
            /* enum type r = */ typecheck(((binary_node_t*)node)->right, e);
            /* assert both types are boolean ... numeric? */
            t = type_from(I1); // I1 boolean
            break;
        case LOGICAL_NOT:
            typecheck(((unary_node_t*)node)->child, e);
            /* assert child t is numeric? all numbers are booleans? */
            t = type_from(I1); // boolean values are represented with I1
            break;
        case BNOT:
        case UPLUS:
        case UMINUS:
        case PRE_INC:
        case PRE_DEC:
        case POST_INC:
        case POST_DEC:
            t = typecheck(((unary_node_t*)node)->child, e);
            // TODO: assert type is numeric...
            break;
        case REFOF:
            t = ref_of(typecheck(((unary_node_t*)node)->child, e));
            break;
        case DEREF:
            t = typecheck(((unary_node_t*)node)->child, e);
            assert(is_type_pointer(t)); // Assert t is a reference
            t = deref(t);
            break;

        case RETURN:
            if (((unary_node_t*)node)->child != NULL)
                t = typecheck(((unary_node_t*)node)->child, e);
            else
                t = type_from(VOID);
            break;
        case CAST:
            t = node->ts;
            typecheck(((unary_node_t*)node)->child, e);
            // TODO: Assert child type is castable to cast type
            break;
        case IF:
            t = type_from(VOID);
            typecheck(((if_node_t*)node)->cond, e);
            // Assert condition is bool or castable to bool? ...
            typecheck(((if_node_t*)node)->thenst, e);
            if (((if_node_t*)node)->elsest != NULL)
                typecheck(((if_node_t*)node)->elsest, e);
            break;
        case CONDITIONAL: {
            typecheck(((if_node_t*)node)->cond, e);
            // TODO: Assert condition is bool or castable to bool? ...
            t = typecheck(((if_node_t*)node)->thenst, e);
            typecheck(((if_node_t*)node)->elsest, e);
            // TODO: Assert then is castable to the same type as else
            break;
        }
        case UNIT:
            t = type_from(VOID);
            break;
    }

    node->ts = t; // assign typechecked type to self
    return t;
}

