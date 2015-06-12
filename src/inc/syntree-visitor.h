#ifndef _INC_SYNTREE_VISITOR_H
#define _INC_SYNTREE_VISITOR_H

#include <inc/syntree.h>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Degenerate an expression to an unary expression.
 */
struct unary_expression *degen_to_unary_expr(struct syntreebasenode *tnode);

#ifdef __cplusplus
}
#endif

#endif
