#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/syntree.h>
#include <inc/syntree-node.h>
#include <inc/dynarr.h>
#include <inc/cgc.h>
#include <inc/type.h>
#include <inc/symtab.h>

#define cgasm_constant_expression cgasm_conditional_expression

static struct expr_val cgasm_conditional_expression(struct cgasm_context *ctx, struct conditional_expression *expr);
static struct expr_val cgasm_cast_expression(struct cgasm_context *ctx, struct cast_expression *expr);

int cgasm_get_int_const_from_expr(struct cgasm_context *ctx, struct expr_val val) {
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

int cgasm_interpret_const_expr(struct cgasm_context *ctx, struct constant_expression *expr) {
	// assert(ctx->const_required); // this not so necessary
	struct expr_val val = cgasm_constant_expression(ctx, expr);
	return cgasm_get_int_const_from_expr(ctx, val);
}

#if 0
static struct type *query_func_type_by_name(struct cgasm_context *ctx, char *name) {
	struct symbol *sym = cgasm_lookup_sym_noabort(ctx, name);
	if (sym == NULL) {
		return NULL;
	}

	if (sym->ctype == NULL) {
		panic("no type defined");
	}

	if (sym->ctype->tag != T_FUNC) {
		panic("%s not defined as function", name);
	}
	return sym->ctype;
}
#endif

void cgasm_change_array_func_to_ptr(struct cgasm_context *ctx, struct expr_val *pval) {
#if 0
	assert(type != NULL);
#else
	if (pval->ctype == NULL) {
		red("cgasm_change_array_func_to_ptr: null type");
		return;
	}
#endif
	struct type *type = expr_val_get_type(*pval);

	if (type->tag == T_ARRAY) {
		assert((pval->type & EXPR_VAL_FLAG_DEREF) == 0);
		assert(pval->type == EXPR_VAL_SYMBOL);
		pval->type = EXPR_VAL_SYMBOL_ADDR;

		struct type *newtype = get_ptr_type(type->subtype);
		type_check_ref(type);
		pval->ctype = newtype;
		register_type_ref(ctx, newtype);
		return;
	}

	if (type->tag == T_FUNC) {
		assert((pval->type & EXPR_VAL_FLAG_DEREF) == 0);
		*pval = cgasm_handle_ampersand(ctx, *pval);
		return;
	}
}

// static struct expr_val cgasm_function_call(struct cgasm_context *ctx, char *funcname, struct argument_expression_list *argu_expr_list) {
static struct expr_val cgasm_function_call(struct cgasm_context *ctx, struct expr_val func, struct argument_expression_list *argu_expr_list) {
	struct dynarr *argu_val_list = dynarr_init();	
	struct expr_val *pval;
	int i;
	char *funcname;

	if (func.type == EXPR_VAL_SYMBOL) {
		funcname = func.sym->name;
	}

	// get func type
	struct type *func_type = expr_val_get_type(func);
	struct type *ret_type = NULL;
	if (func_type == NULL) {
		red("function %s not declared", funcname);
		ret_type = get_int_type(); // the default
	} else {
		if (func_type->tag == T_PTR) { // handle func ptr
			func_type = func_type->subtype;
		}

		if (func_type->tag != T_FUNC) {
			panic("require func type %d", func_type->tag);
		}
		ret_type = func_type->func.retype;
	}

#if 0
	if (func_type != NULL) {
		panic("need check argument type: maybe need do conversion");
	}
#endif

	DYNARR_FOREACH_BEGIN(argu_expr_list->list, assignment_expression, each);
		pval = mallocz(sizeof(*pval));
		*pval = cgasm_assignment_expression(ctx, each);
		cgasm_change_array_func_to_ptr(ctx, pval);
		dynarr_add(argu_val_list, pval);
	DYNARR_FOREACH_END();

	// push arguments in reverse order
	for (i = dynarr_size(argu_val_list) - 1; i >= 0; i--) {
		cgasm_push_val(ctx, *(struct expr_val *) dynarr_get(argu_val_list, i));
	}

	// emit call 
	if (func.type == EXPR_VAL_SYMBOL) {
		cgasm_println(ctx, "call %s", func.sym->name);
	} else {
		int reg = REG_EAX;
		cgasm_load_val_to_reg(ctx, func, reg);
		cgasm_println(ctx, "call *%%%s", get_reg_str_code(reg));
	}

	// pop
	cgasm_println(ctx, "addl $%d, %%esp", dynarr_size(argu_val_list) * 4); // XXX assume each argument takes 4 bytes right now

	// cleanup
	DYNARR_FOREACH_BEGIN(argu_val_list, expr_val, each);
		free(each);
	DYNARR_FOREACH_END();

	dynarr_destroy(argu_val_list);

	if (ret_type->tag == T_VOID) {
		return void_expr_val();
	} else {
		struct expr_val retval;
		// TODO current simple implementation is always return a temp holding eax
		if (ret_type->size != 4) {
			panic("return type size other than 4 is not supported yet");
		}
		retval = cgasm_alloc_temp_var(ctx, ret_type); 
		cgasm_store_reg_to_mem(ctx, REG_EAX, retval); // TODO consider returning a long long or struct
		return retval;
	}
}

static struct expr_val cgasm_primary_expression(struct cgasm_context *ctx, struct primary_expression *expr) {
	if (expr->str != NULL) {
		return cgasm_register_str_literal(ctx, expr->str);
	} else if (expr->id != NULL) {
		struct expr_val ret = symbol_expr_val(cgasm_lookup_sym(ctx, expr->id));
		#if 0 // this is not true for func call
		if (ret.ctype->size < 0) {
			panic("non complement type for symbol used in primary expression");
		} 
		#endif
		return ret;
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
	#if 0
	struct postfix_expression_suffix *suff;
	char *id;
	// function call TODO unify this into the general processing
	if ((id = expr->prim_expr->id) != NULL && dynarr_size(expr->suff_list) == 1 && (suff = dynarr_get(expr->suff_list, 0))->arg_list != NULL) {
		return cgasm_function_call(ctx, id, suff->arg_list);
	}
	#endif

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
		} else if (each->dot_id) {
			result_val = cgasm_handle_ampersand(ctx, result_val);
			result_val = cgasm_handle_ptr_op(ctx, result_val, each->dot_id);
		} else if (each->ptr_id) {
			result_val = cgasm_handle_ptr_op(ctx, result_val, each->ptr_id);
		} else if (each->arg_list) {
			result_val = cgasm_function_call(ctx, result_val, each->arg_list);
		} else {
			panic("can not reach here");
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
		struct type *type = parse_type_from_specifier_qualifier_list(ctx, expr->sizeof_type->sqlist);
		int type_size;
		if (expr->sizeof_type->declarator != NULL) {
			char *id = NULL;
			type = parse_type_from_declarator(ctx, type, expr->sizeof_type->declarator, &id);
			if (id != NULL) {
				panic("sizeof can not used with (concrete) declarator");
			}
		}
		register_type_ref(ctx, type);
		type_size = type_get_size(type);
		if (type_size < 0) {
			panic("sizeof returns negative size type");
		}
		return int_const_expr_val(type_size);
	} else if (expr->inc_unary) {
		struct expr_val val = cgasm_unary_expression(ctx, expr->inc_unary);
		struct expr_val ret = cgasm_handle_pre_inc(ctx, val);
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
	struct expr_val val;
	if (expr->unary_expr != NULL) {
		val = cgasm_unary_expression(ctx, expr->unary_expr);
	} else {
		fprintf(stderr, "\033[31mtype casting is not implemented yet\033[0m\n");
		val = cgasm_cast_expression(ctx, expr->cast_expr);
	}
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

struct expr_val cgasm_logical_or_expression(struct cgasm_context *ctx, struct logical_or_expression *expr) {
	return CGASM_BINARY_OP_EXPR(ctx, expr->and_expr_list, logical_and_expression, NULL, TOK_LOGIC_OR);
}

// right assoc
static struct expr_val cgasm_conditional_expression_rec(struct cgasm_context *ctx, struct dynarr *inner_expr_list, int inner_expr_ind, struct dynarr *or_expr_list, int or_expr_ind) {
	int or_size = dynarr_size(or_expr_list);
	struct expr_val left_most;

	assert(or_expr_ind < or_size);
	assert(or_size - or_expr_ind == dynarr_size(inner_expr_list) - inner_expr_ind + 1);

	left_most = cgasm_logical_or_expression(ctx, dynarr_get(or_expr_list, or_expr_ind));
	if (or_expr_ind == or_size - 1) {
		return left_most;
	}

	// check the case that the cond expr is constant
	if (left_most.type == EXPR_VAL_CONST_VAL) {
		if (const_token_is_nonzero(left_most.const_val)) {
			return cgasm_expression(ctx, dynarr_get(inner_expr_list, inner_expr_ind));
		} else {
			return cgasm_conditional_expression_rec(ctx, inner_expr_list, inner_expr_ind + 1, or_expr_list, or_expr_ind + 1);
		}
	} else {
		struct expr_val temp = cgasm_alloc_temp_var(ctx, get_int_type()); // XXX assume integer type
		cgasm_handle_conditional(ctx, left_most, inner_expr_list, inner_expr_ind, or_expr_list, or_expr_ind + 1, temp);
		return temp;
	}
}

static struct expr_val cgasm_conditional_expression(struct cgasm_context *ctx, struct conditional_expression *expr) {
	return cgasm_conditional_expression_rec(ctx, expr->inner_expr_list, 0, expr->or_expr_list, 0);
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
