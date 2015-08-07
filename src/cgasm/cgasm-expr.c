#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/syntree.h>
#include <inc/syntree-node.h>
#include <inc/dynarr.h>
#include <inc/cgc.h>
#include <inc/type.h>
#include <inc/symtab.h>
#include <inc/ll-support.h>

#define cgasm_constant_expression cgasm_conditional_expression

static struct expr_val cgasm_conditional_expression(struct cgasm_context *ctx, struct conditional_expression *expr);
static struct expr_val cgasm_cast_expression(struct cgasm_context *ctx, struct cast_expression *expr);

long long cgasm_get_ll_const_from_expr(struct cgasm_context *ctx, struct expr_val val) {
	if (val.type != EXPR_VAL_CONST_VAL) {
		panic("constant value required");
	}

	union token const_tok = val.const_val;
	assert(const_tok.tok_tag == TOK_CONSTANT_VALUE);
	if (!(const_tok.const_val.flags & CONST_VAL_TOK_LONG_LONG)) {
		panic("long long constant required");
	}
	return const_tok.const_val.llval;
}

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
	struct type *type = expr_val_get_type(*pval);

	if (type->tag == T_ARRAY) {
		if (expr_val_has_deref_flag(*pval)) {
			*pval = expr_val_remove_deref_flag(*pval);	
			pval->ctype = get_ptr_type(type->subtype);
			register_type_ref(ctx, pval->ctype);
			return;
		}

		if (pval->type == EXPR_VAL_SYMBOL) {
			pval->type = EXPR_VAL_SYMBOL_ADDR;
		} else {
			panic("expr val type %d", pval->type);
		}

		struct type *newtype = get_ptr_type(type->subtype);
		type_check_ref(type);
		pval->ctype = newtype;
		register_type_ref(ctx, newtype);
		return;
	}

	if (type->tag == T_FUNC) {
		// the following assertion is not correct becaues of this caes
		//   f(*st->func_ptr)
		// assert((pval->type & EXPR_VAL_FLAG_DEREF) == 0); 
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

	bool func_sym = func.type == EXPR_VAL_SYMBOL && func.sym->ctype->tag == T_FUNC;

	if (func_sym) {
		funcname = func.sym->name;
	}

	// get func type
	struct type *func_type = expr_val_get_type(func);
	assert(func_type != NULL); // does not support undeclared func right now

	struct type *ret_type = NULL;
	#if 0
	if (func_type == NULL) {
		red("function %s not declared", funcname);
		ret_type = get_int_type(); // the default
	} else  
	#endif
	{
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

	// TODO does not support implicit type convertsion yet!! This may cause error
	int arg_space_size = 0;
	DYNARR_FOREACH_BEGIN(argu_expr_list->list, assignment_expression, each);
		pval = mallocz(sizeof(*pval));
		*pval = cgasm_assignment_expression(ctx, each);
		cgasm_change_array_func_to_ptr(ctx, pval);
		dynarr_add(argu_val_list, pval);
		arg_space_size += type_get_size(expr_val_get_type(*pval));
	DYNARR_FOREACH_END();

	// push arguments in reverse order
	for (i = dynarr_size(argu_val_list) - 1; i >= 0; i--) {
		cgasm_push_val(ctx, *(struct expr_val *) dynarr_get(argu_val_list, i));
	}

	// emit call 
	if (func_sym) {
		cgasm_println(ctx, "call %s", func.sym->name);
	} else {
		int reg = REG_EAX;
		cgasm_load_val_to_reg(ctx, func, reg);
		cgasm_println(ctx, "call *%%%s", get_reg_str_code(reg));
	}

	// pop
	cgasm_println(ctx, "addl $%d, %%esp", arg_space_size); 

	// cleanup
	DYNARR_FOREACH_BEGIN(argu_val_list, expr_val, each);
		free(each);
	DYNARR_FOREACH_END();

	dynarr_destroy(argu_val_list);

	if (ret_type->tag == T_VOID) {
		return void_expr_val();
	} else if (ret_type->tag == T_LONG_LONG) {
		struct expr_val retval = cgasm_alloc_temp_var(ctx, ret_type);
		cgasm_store_reg2_to_ll_temp(ctx, REG_EAX, REG_EDX, retval);
		return retval;
	} else if (ret_type->size <= 4) {
		struct expr_val retval = cgasm_alloc_temp_var(ctx, ret_type); 

		// even if we return a short, we can copy the entire EAX. The high order bytes
		// will be cleared later
		cgasm_store_reg_to_mem(ctx, REG_EAX, retval); 
		return retval;
	} else {
		type_dump(ret_type, 4);
		panic("unsupported return type, %s", funcname == NULL ? "(nil)" : funcname);
	}
}

static struct expr_val cgasm_primary_expression(struct cgasm_context *ctx, struct primary_expression *expr) {
	if (expr->str != NULL) {
		return cgasm_register_str_literal(ctx, expr->str);
	} else if (expr->id != NULL) {
		struct symbol *sym = cgasm_lookup_sym_noabort(ctx, expr->id);
		if (sym != NULL) {
			return symbol_expr_val(sym);
		}
		struct expr_val ret;
		if (check_builtin_symbol(ctx, expr->id, &ret)) {
			return ret;
		}

		panic("symbol undefined: %s", expr->id);
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

static struct type *cgasm_type_name(struct cgasm_context *ctx, struct type_name *type_name) {
	struct type *type = parse_type_from_specifier_qualifier_list(ctx, type_name->sqlist);
	if (type_name->declarator != NULL) {
		char *id = NULL;
		type = parse_type_from_declarator(ctx, type, type_name->declarator, &id);
		if (id != NULL) {
			panic("type_name should not contains identifier");
		}
	}
	// register_type_ref(ctx, type);
	return type;
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
		struct type *type = expr_val_get_type(val);
		return int_const_expr_val(type_get_size(type));
	} else if (expr->sizeof_type) {
		struct type *type = cgasm_type_name(ctx, expr->sizeof_type);
		int type_size = type_get_size(type);
		if (type_size < 0) {
			panic("sizeof returns negative size type");
		}
		type_check_ref(type);
		return int_const_expr_val(type_size);
	} else if (expr->inc_unary) {
		struct expr_val val = cgasm_unary_expression(ctx, expr->inc_unary);
		struct expr_val ret = cgasm_handle_pre_inc(ctx, val);
		return ret;
	} else if (expr->dec_unary) {
		struct expr_val val = cgasm_unary_expression(ctx, expr->dec_unary);
		struct expr_val ret = cgasm_handle_pre_dec(ctx, val);
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
		struct type *newtype = cgasm_type_name(ctx, expr->type_name);
		val = cgasm_cast_expression(ctx, expr->cast_expr);
		val = type_convert(ctx, val, newtype);
		register_type_ref(ctx, newtype);
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
		// we do not allocate temp var here since we know the type later
		return cgasm_handle_conditional(ctx, left_most, inner_expr_list, inner_expr_ind, or_expr_list, or_expr_ind + 1, void_expr_val());
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
