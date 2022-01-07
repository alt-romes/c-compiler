#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <typecheck.h>
#include <debug.h>

type_t typecheck(struct node* node, struct environment* e) {
    type_t t;
    switch (node->type) {
        case FUNCTION: {

            debug("Typecheck: function");

            /* this doesn't need to be, only make sure that its castable from one to another assert((t = node->ts) == typecheck(((function_node_t*)node)->body, e)); */
            t = ((function_node_t*)node)->decl.ts; // function type is its own declarator type

            debug("Typecheck: function, begin scope");
            environment_t* scope_env = beginScope(e);

            function_type_t fts = (function_type_t)t;
            int argc = fts->args->size;

            for (int i = 0; i < argc; i++)
                assoc(scope_env, fts->args->args[i].id,
                        (union association_v){ .type = fts->args->args[i].ts });

            typecheck(((function_node_t*)node)->body, scope_env);

            endScope(scope_env); // free the scope environment and its association array

            break;
        }
        case ID:
            debugf1("Typecheck: ID %s", ((id_node_t*)node)->value);
            // Set this node's type to the one found in the environment
            t = find(e, ((id_node_t*)node)->value).type;
            debug_type("ID has type", t);
            break;
        case GLOBAL_BLOCK:
        case BLOCK: {

            debug("Typecheck: block");

            environment_t* scope_env = beginScope(e);

            block_node_t* bnode = (block_node_t*)node;
            declaration_list_t* dae = bnode->declaration_list;  // this id->ast_node environment is freed when the whole ast is freed
                                                                // NOTE: the ids will be freed with the ast (they were allocated with it, so they should be deallocated with it)

            // For each declaration in this scope create an association in this scope's evaluation environment
            for (int i = 0; i < dae->size; i++) {
                debugf1("Typecheck: block declarator %s", dae->declarations[i].id);
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

            debug("Typecheck: block done");

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
            debug("Typecheck: assign");
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
            debug("Typecheck: LT, GT, LE, or GE");
            debug("Typecheck: left");
            type_t l = typecheck(((binary_node_t*)node)->left, e);
            debug("Typecheck: right");
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
            debug("Typecheck: ref of");
            t = ref_of(typecheck(((unary_node_t*)node)->child, e));
            break;
        case DEREF:
            debug("Typecheck: deref");
            t = typecheck(((unary_node_t*)node)->child, e);
            assert(is_type_pointer(t)); // Assert t is a reference
            t = deref(t);
            break;

        case RETURN:
            debug("Typecheck: return");
            if (((unary_node_t*)node)->child != NULL) {
                t = typecheck(((unary_node_t*)node)->child, e);
            }
            else
                t = type_from(VOID);
            break;
        case CAST:
            debug("Typecheck: cast");
            t = node->ts;
            typecheck(((unary_node_t*)node)->child, e);
            // TODO: Assert child type is castable to cast type
            break;
        case IF:
            debug("Typecheck: if");
            t = type_from(VOID);
            debug("Typecheck: cond");
            typecheck(((if_node_t*)node)->cond, e);
            // Assert condition is bool or castable to bool? ...
            debug("Typecheck: then");
            typecheck(((if_node_t*)node)->thenst, e);
            debug("Typecheck: else?");
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
        case CALL:
            debug("Typecheck: Call");
            t = typecheck(((binary_node_t*)node)->left, e); // typecheck function name and get function type as a result
            debug("Typecheck: Call arguments");
            assert(((function_type_t)t)->args->size == ((statement_list_t*)((binary_node_t*)node)->right)->size);
            for (int i = 0; i < ((function_type_t)t)->args->size; i++) {
                // assert argument passed is castable to argument expected
                statement_list_t* args_list = (statement_list_t*)((binary_node_t*)node)->right;
                typecheck(args_list->statements[i], e);
            }
            t = ((function_type_t)t)->ret; // call type is the return type of the function called
            debug_type("Typecheck: call type is ret type:", t);
            break;
    }

    node->ts = t; // assign typechecked type to self
    return t;
}

