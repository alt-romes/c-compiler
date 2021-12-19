#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "typecheck.h"

enum type typecheck(struct node* node, struct environment* e) {
    enum type t;
    switch (node->type) {
        case FUNCTION: {
            // TODO ... 
            /* this doesn't need to be, only make sure that its castable from one to another assert((t = node->ts) == typecheck(((function_node_t*)node)->body, e)); */
            t = node->ts; // function type is its return value... quite wrong but ..
            typecheck(((function_node_t*)node)->body, e);
            break;
        }
        case ID: {
            // Set this node's type to the one found in the environment
            t = find(e, ((id_node_t*)node)->value).type;
            break;
        }
        case BLOCK: {

            environment_t* scope_env = beginScope(e);

            block_node_t* bnode = (block_node_t*)node;
            declaration_list_t* dae = bnode->declaration_list;  // this id->ast_node environment is freed when the whole ast is freed
                                                                // NOTE: the ids will be freed with the ast (they were allocated with it, so they should be deallocated with it)

            // For each declaration in this scope create an association in this scope's evaluation environment
            for (int i = 0; i < dae->size; i++) {
                /* dae->declarations[i].node->ts = dae->declarations[i].et; this isn't needed, a cast should be made when associating a value to an identifier */
                typecheck(dae->declarations[i].node, scope_env); // Must still typecheck all declaration values
                /* assert(tychkty == dae->declarations[i].node->ts); No longer true, the value doesn't need to have the same time because it'll be casted on assignment */
                assoc(scope_env, dae->declarations[i].id, (union association_v){ .type = dae->declarations[i].et });
            }

            t = typecheck(((block_node_t*)node)->body, scope_env);

            endScope(scope_env); // free the scope environment and its association array

            break;
        }
        case NUM:
            // TODO: Just Min required value to hold this number, ignore node->ts
            /* t = node->ts == 0 ? INT : node->ts; // Use already assigned type for num (on declaration) if available */
            t = INT;
            break;
        case ADD:
        case SUB:
        case MUL:
        case DIV: {
            enum type l = typecheck(((binary_node_t*)node)->left, e);
            enum type r = typecheck(((binary_node_t*)node)->right, e);
            // TODO: assert both types are "addable"... (aren't they all?)
            t = type_compare(l, r) < 0 ? r : l;
            break;
        }
        case EQ:
        case NE: {
            enum type l = typecheck(((binary_node_t*)node)->left, e);
            enum type r = typecheck(((binary_node_t*)node)->right, e);
            // Correction to the below: types can be ints with different size that will be promoted
            /* assert(type_compare(l, r) == 0); // Both types must be the same */
            t = CHAR; // Return boolean encoded in char
            break;
        }
        case LOGICAL_NOT: {
            t = typecheck(((unary_node_t*)node)->child, e);
            /* assert(t == NUM); assert t is numeric? all numbers are booleans? */
            break;
        }
        case UMINUS: {
            t = typecheck(((unary_node_t*)node)->child, e);
            // TODO: assert type is numeric...
            break;
        }
    }

    node->ts = t; // assign typechecked type to self
    return t;
}

