#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/syntree.h>
#include <inc/syntree-node.h>
#include <inc/dynarr.h>

static struct expr_val cgasm_assignment_expression(struct cgasm_context *ctx, struct assignment_expression *expr);
static struct expr_val cgasm_cast_expression(struct cgasm_context *ctx, struct cast_expression *expr);

static struct expr_val cgasm_function_call(struct cgasm_context *ctx, char *funcname, struct argument_expression_list *argu_expr_list) {
	struct dynarr *argu_val_list = dynarr_init();	
	struct expr_val *pval;
	int i;
	DYNARR_FOREACH_BEGIN(argu_expr_list->list, assignment_expression, each);
		pval = malloc(sizeof(*pval));
		*pval = cgasm_assignment_expression(ctx, each);
		dynarr_add(argu_val_list, pval);
	DYNARR_FOREACH_END();

	// push arguments in reverse order
	for (i = dynarr_size(argu_val_list) - 1; i >= 0; i--) {
		cgasm_push_val(ctx, *(struct expr_val *) dynarr_get(argu_val_list, i));
	}

	// emit call 
	cgasm_println(ctx, "call %s", funcname);

	// pop
	cgasm_println(ctx, "addl %%esp, %d", dynarr_size(argu_val_list) * 4); // XXX assume each argument takes 4 bytes right now

	// cleanup
	DYNARR_FOREACH_BEGIN(argu_val_list, expr_val, each);
		free(each);
	DYNARR_FOREACH_END();

out:
	dynarr_destroy(argu_val_list);
}

static struct expr_val cgasm_primary_expression(struct cgasm_context *ctx, struct primary_expression *expr) {
	if (expr->str != NULL) {
		return cgasm_register_str_literal(ctx, expr->str);
	} else if (expr->id != NULL) {
		return symbol_expr_val(cgasm_lookup_sym(ctx, expr->id));
	} else if (expr->const_val_tok.tok_tag != TOK_UNDEF) {
		return const_expr_val(expr->const_val_tok);
	} else {
		panic("ni");
	}
}

/**
 * XXX this method only handle several special cases right now.
 */
static struct expr_val cgasm_postfix_expression(struct cgasm_context *ctx, struct postfix_expression *expr) {
	struct postfix_expression_suffix *suff;
	char *id;
	// function call
	if ((id = expr->prim_expr->id) != NULL && dynarr_size(expr->suff_list) == 1 && (suff = dynarr_get(expr->suff_list, 0))->arg_list != NULL) {
		return cgasm_function_call(ctx, id, suff->arg_list);
	}

	// primary expression
	if (dynarr_size(expr->suff_list) == 0) {
		return cgasm_primary_expression(ctx, expr->prim_expr);
	}
	panic("ni");
}

static struct expr_val cgasm_unary_expression(struct cgasm_context *ctx, struct unary_expression *expr) {
	if (expr->postfix_expr != NULL) {
		return cgasm_postfix_expression(ctx, expr->postfix_expr);
	} else if (expr->unary_op_cast != NULL) {
		struct expr_val val = cgasm_cast_expression(ctx, expr->unary_op_cast);
		return cgasm_handle_unary_op(ctx, expr->unary_op, val);
	} else {
		panic("ni");
	}
}

static struct expr_val cgasm_cast_expression(struct cgasm_context *ctx, struct cast_expression *expr) {
	struct expr_val val = cgasm_unary_expression(ctx, expr->unary_expr);
	// TODO handle cast
	return val;
}

/*
 * XXX either get op from oplist or use single_op
 */
#define CGASM_BINARY_OP_EXPR(ctx, subexpr_list, subexpr_type, oplist, single_op) ({ \
	struct expr_val accum; \
	int i; \
	accum = cgasm_ ## subexpr_type(ctx, dynarr_get(subexpr_list, 0)); \
	for (i = 1; i < dynarr_size(subexpr_list); i++) { \
		struct expr_val extra = cgasm_ ## subexpr_type(ctx, dynarr_get(subexpr_list, i)); \
		int op = oplist == NULL ? single_op : (int) (long) dynarr_get(oplist, i - 1); \
		accum = cgasm_handle_binary_op(ctx, op, accum, extra); \
	} \
	accum; \
})

static struct expr_val cgasm_multiplicative_expression(struct cgasm_context *ctx, struct multiplicative_expression *expr) {
	return CGASM_BINARY_OP_EXPR(ctx, expr->cast_expr_list, cast_expression, expr->oplist, TOK_UNDEF);
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
	return CGASM_BINARY_OP_EXPR(ctx, expr->or_expr_list, inclusive_or_expression, NULL, TOK_LOGIC_AND);
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
	int i;
	for (i = dynarr_size(expr->unary_expr_list) - 1; i >= 0; i--) {
		struct expr_val un = cgasm_unary_expression(ctx, dynarr_get(expr->unary_expr_list, i));
		int op = (int) (long) dynarr_get(expr->oplist, i);
		val = cgasm_handle_assign_op(ctx, un, val, op);
	}
	return val;
}

struct expr_val cgasm_expression(struct cgasm_context *ctx, struct expression *expr) {
	struct expr_val ret;
	DYNARR_FOREACH_BEGIN(expr->darr, assignment_expression, each);	
		ret = cgasm_assignment_expression(ctx, each); // the last one win
	DYNARR_FOREACH_END();

	return ret;
}
