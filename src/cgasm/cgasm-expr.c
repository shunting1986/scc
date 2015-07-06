#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/syntree.h>
#include <inc/syntree-node.h>
#include <inc/dynarr.h>

static struct expr_val cgasm_cast_expression(struct cgasm_context *ctx, struct cast_expression *expr) {
	panic("ni");
}

/*
 * XXX either get op from oplist or use single_op
 */
#define CGASM_BINARY_OP_EXPR(ctx, subexpr_list, subexpr_type, oplist, single_op) ({ \
	struct expr_val accum; \
	int i; \
	accum = cgasm_ ## subexpr_type(ctx, dynarr_get(subexpr_list, 0)); \
	for (i = 1; i < dynarr_size(subexpr_list); i++) { \
		panic("ni"); \
	} \
	accum; \
})

static struct expr_val cgasm_multiplicative_expression(struct cgasm_context *ctx, struct multiplicative_expression *expr) {
	return CGASM_BINARY_OP_EXPR(ctx, expr->cast_expr_list, cast_expression, expr-oplist, TOK_UNDEF);
}

static struct expr_val cgasm_additive_expression(struct cgasm_context *ctx, struct additive_expression *expr) {
	return CGASM_BINARY_OP_EXPR(ctx, expr->mul_expr_list, multiplicative_expression, expr->oplist, TOK_UNDEF);
}

static struct expr_val cgasm_shift_expression(struct cgasm_context *ctx, struct shift_expression *expr) {
	return CGASM_BINARY_OP_EXPR(ctx, expr->add_expr_list, additive_expression, expr->oplist, TOK_UNDEF);
}

static struct expr_val cgasm_relational_expression(struct cgasm_context *ctx, struct relational_expression *expr) {
	return CGASM_BINARY_OP_EXPR(ctx, expr->shift_expr_list, shift_expression, expr->oplist, TOK_UNDEF);
}

static struct expr_val cgasm_equality_expression(struct cgasm_context *ctx, struct equality_expression *expr) {
	return CGASM_BINARY_OP_EXPR(ctx, expr->rel_expr_list, relational_expression, expr->oplist, TOK_UNDEF);
}

static struct expr_val cgasm_and_expression(struct cgasm_context *ctx, struct and_expression *expr) {
	return CGASM_BINARY_OP_EXPR(ctx, expr->eq_expr_list, equality_expression, NULL, TOK_AMPERSAND);
}

static struct expr_val cgasm_exclusive_or_expression(struct cgasm_context *ctx, struct exclusive_or_expression *expr) {
	return CGASM_BINARY_OP_EXPR(ctx, expr->and_expr_list, and_expression, NULL, TOK_XOR);
}

static struct expr_val cgasm_inclusive_or_expression(struct cgasm_context *ctx, struct inclusive_or_expression *expr) {
	return CGASM_BINARY_OP_EXPR(ctx, expr->xor_expr_list, exclusive_or_expression, NULL, TOK_VERT_BAR);
}

static struct expr_val cgasm_logical_and_expression(struct cgasm_context *ctx, struct logical_and_expression *expr) {
	return CGASM_BINARY_OP_EXPR(ctx, expr->or_expr_list, inclusive_or_expression, NULL, TOK_LOGICAL_AND);
}

static struct expr_val cgasm_logical_or_expression(struct cgasm_context *ctx, struct logical_or_expression *expr) {
	return CGASM_BINARY_OP_EXPR(ctx, expr->and_expr_list, logical_and_expression, NULL, TOK_LOGIC_OR);
}

// right assoc
static struct expr_val cgasm_conditional_expression(struct cgasm_context *ctx, struct conditional_expression *expr) {
	int or_size = dynarr_size(expr->or_expr_list);
	struct expr_val right_most;
	int i;

	assert(or_size > 0);
	assert(or_size == dynarr_size(expr->inner_expr_list) + 1);

	right_most = cgasm_logical_or_expression(ctx, dynarr_get(expr->or_expr_list, or_size - 1));

	for (i = or_size - 2; i >= 0; i--) {
		panic("ni");
	}
	return right_most;
}

static struct expr_val cgasm_assignment_expression(struct cgasm_context *ctx, struct assignment_expression *expr) {
	struct expr_val val = cgasm_conditional_expression(ctx, expr->cond_expr);
	if (dynarr_size(expr->unary_expr_list) == 0) {
		return val;
	}
	panic("ni");
}

struct expr_val cgasm_expression(struct cgasm_context *ctx, struct expression *expr) {
	DYNARR_FOREACH_BEGIN(expr->darr, assignment_expression, each);	
		cgasm_assignment_expression(ctx, each);
	DYNARR_FOREACH_END();
}
