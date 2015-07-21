#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/syntree.h>
#include <inc/syntree-node.h>
#include <inc/dynarr.h>

#define cgasm_constant_expression cgasm_conditional_expression

static struct expr_val cgasm_conditional_expression(struct cgasm_context *ctx, struct conditional_expression *expr);
static struct expr_val cgasm_cast_expression(struct cgasm_context *ctx, struct cast_expression *expr);

int cgasm_interpret_const_expr(struct cgasm_context *ctx, struct constant_expression *expr) {
	assert(ctx->const_required);
	struct expr_val val = cgasm_constant_expression(ctx, expr);

	if (val.type != EXPR_VAL_CONST_VAL) {
		panic("constant value required");
	}

	union token const_tok = val.const_val;
	assert(const_tok.tok_tag == TOK_CONSTANT_VALUE);
	if (!(const_tok.const_val.flags & CONST_VAL_TOK_INTEGER)) {
		panic("integer constant required");
	}
	return const_tok.const_val.ival;
}

static struct expr_val cgasm_function_call(struct cgasm_context *ctx, char *funcname, struct argument_expression_list *argu_expr_list) {
	struct dynarr *argu_val_list = dynarr_init();	
	struct expr_val *pval, retval;
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
	cgasm_println(ctx, "addl $%d, %%esp", dynarr_size(argu_val_list) * 4); // XXX assume each argument takes 4 bytes right now

	// cleanup
	DYNARR_FOREACH_BEGIN(argu_val_list, expr_val, each);
		free(each);
	DYNARR_FOREACH_END();

	dynarr_destroy(argu_val_list);

	// TODO current simple implementation is always return a temp holding eax
	retval = cgasm_alloc_temp_var(ctx);
	cgasm_store_reg_to_mem(ctx, REG_EAX, retval);
	return retval;
}

static struct expr_val cgasm_primary_expression(struct cgasm_context *ctx, struct primary_expression *expr) {
	if (expr->str != NULL) {
		return cgasm_register_str_literal(ctx, expr->str);
	} else if (expr->id != NULL) {
		return symbol_expr_val(cgasm_lookup_sym(ctx, expr->id));
	} else if (expr->const_val_tok.tok_tag != TOK_UNDEF) {
		return const_expr_val(expr->const_val_tok);
	} else if (expr->expr != NULL) {
		return cgasm_expression(ctx, expr->expr);
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
	// function call TODO unify this into the general processing
	if ((id = expr->prim_expr->id) != NULL && dynarr_size(expr->suff_list) == 1 && (suff = dynarr_get(expr->suff_list, 0))->arg_list != NULL) {
		return cgasm_function_call(ctx, id, suff->arg_list);
	}

	// first, we get expr_val from the primary expression
	struct expr_val result_val = cgasm_primary_expression(ctx, expr->prim_expr);
	DYNARR_FOREACH_BEGIN(expr->suff_list, postfix_expression_suffix, each);
		if (each->is_inc) {
			result_val = cgasm_handle_post_inc(ctx, result_val);
		} else if (each->is_dec) {
			result_val = cgasm_handle_post_dec(ctx, result_val);
		} else if (each->ind) {
			struct expr_val ind_val = cgasm_expression(ctx, each->ind);
			result_val = cgasm_handle_index_op(ctx, result_val, ind_val);
		} else {
			panic("ni");
		}
	DYNARR_FOREACH_END();
	return result_val;
}

static struct expr_val cgasm_unary_expression(struct cgasm_context *ctx, struct unary_expression *expr) {
	if (expr->postfix_expr != NULL) {
		return cgasm_postfix_expression(ctx, expr->postfix_expr);
	} else if (expr->unary_op_cast != NULL) {
		struct expr_val val = cgasm_cast_expression(ctx, expr->unary_op_cast);
		return cgasm_handle_unary_op(ctx, expr->unary_op, val);
	} else if (expr->sizeof_expr) {
		// TODO we should not interp the expression. Retrieve the type is enough
		struct expr_val val = cgasm_unary_expression(ctx, expr->sizeof_expr);
		return int_const_expr_val(type_get_size(val.ctype));
	} else if (expr->sizeof_type) {
		// TODO: abstract declarator is not handled yet
		struct type *type = parse_type_from_specifier_qualifier_list(ctx, expr->sizeof_type->sqlist);
		return int_const_expr_val(type_get_size(type));
	} else if (expr->inc_unary) {
		struct expr_val val = cgasm_unary_expression(ctx, expr->inc_unary);
		struct expr_val ret = cgasm_handle_pre_inc(ctx, val);
		type_put(val.ctype);
		return ret;
	} else if (expr->dec_unary) {
		struct expr_val val = cgasm_unary_expression(ctx, expr->dec_unary);
		struct expr_val ret = cgasm_handle_pre_dec(ctx, val);
		type_put(val.ctype);
		return ret;
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
		int op = oplist == NULL ? single_op : (int) (long) dynarr_get(oplist, i - 1); \
		if (op != TOK_LOGIC_AND && op != TOK_LOGIC_OR) { \
			struct expr_val extra = cgasm_ ## subexpr_type(ctx, dynarr_get(subexpr_list, i)); \
			accum = cgasm_handle_binary_op(ctx, op, accum, extra); \
		} else { \
			accum = cgasm_handle_binary_op_lazy(ctx, op, accum, dynarr_get(subexpr_list, i)); \
		} \
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
	struct expr_val left_most;

	assert(or_size > 0);
	assert(or_size == dynarr_size(expr->inner_expr_list) + 1);

	left_most = cgasm_logical_or_expression(ctx, dynarr_get(expr->or_expr_list, 0));
	if (or_size == 1) {
		return left_most;
	}

	struct expr_val temp = cgasm_alloc_temp_var(ctx);

	cgasm_handle_conditional(ctx, left_most, expr->inner_expr_list, 0, expr->or_expr_list, 1, temp);
	return temp;
}

struct expr_val cgasm_assignment_expression(struct cgasm_context *ctx, struct assignment_expression *expr) {
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

struct expr_val cgasm_eval_expr(struct cgasm_context *ctx, struct syntreebasenode *rawexpr) {
	switch (rawexpr->nodeType) {
	case INCLUSIVE_OR_EXPRESSION:
		return cgasm_inclusive_or_expression(ctx, (void *) rawexpr);
	case LOGICAL_AND_EXPRESSION:
		return cgasm_logical_and_expression(ctx, (void *) rawexpr);
	default:
		panic("ni %s", node_type_str(rawexpr->nodeType));
	}
}
