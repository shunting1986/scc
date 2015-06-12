#include <inc/syntree-visitor.h>
#include <inc/util.h>

// this is typically used to degenerate an conditional expression to an
// unary expression
struct unary_expression *degen_to_unary_expr(struct syntreebasenode *tnode) {

// no need to put break after DEGEN_ONE_LEVEL
#define DEGEN_ONE_LEVEL(real_type, list_name) do { \
	struct real_type *container = (struct real_type *) tnode;	\
	if (dynarr_size(container->list_name) == 1) { \
		void *item = dynarr_get(container->list_name, 0); \
		real_type ## _destroy(container); \
		return degen_to_unary_expr(item); \
	} \
	goto err; \
} while (0)

	switch (tnode->nodeType) {
	case CONDITIONAL_EXPRESSION:
		DEGEN_ONE_LEVEL(conditional_expression, or_expr_list); 
	case LOGICAL_OR_EXPRESSION: 
		DEGEN_ONE_LEVEL(logical_or_expression, and_expr_list);
	case LOGICAL_AND_EXPRESSION: 
		DEGEN_ONE_LEVEL(logical_and_expression, or_expr_list);
	case INCLUSIVE_OR_EXPRESSION:
		DEGEN_ONE_LEVEL(inclusive_or_expression, xor_expr_list);
	case EXCLUSIVE_OR_EXPRESSION:
		DEGEN_ONE_LEVEL(exclusive_or_expression, and_expr_list);
	case AND_EXPRESSION:
		DEGEN_ONE_LEVEL(and_expression, eq_expr_list);
	case EQUALITY_EXPRESSION:
		DEGEN_ONE_LEVEL(equality_expression, rel_expr_list);
	case RELATIONAL_EXPRESSION:
		DEGEN_ONE_LEVEL(relational_expression, shift_expr_list);
	case SHIFT_EXPRESSION:
		DEGEN_ONE_LEVEL(shift_expression, add_expr_list);
	case ADDITIVE_EXPRESSION:
		DEGEN_ONE_LEVEL(additive_expression, mul_expr_list);
	case MULTIPLICATIVE_EXPRESSION:
		DEGEN_ONE_LEVEL(multiplicative_expression, cast_expr_list);
	case CAST_EXPRESSION: {
		struct cast_expression *cast_expr = (struct cast_expression *) tnode;
		if (cast_expr->unary_expr != NULL) {
			return cast_expr->unary_expr;
		}
		goto err;
	}
	default:
err:
		panic("unary_expression required, was %d", tnode->nodeType);
	}

#undef DEGEN_ONE_LEVEL
}
