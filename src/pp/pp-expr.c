#include <inc/pp.h>
#include <inc/util.h>
#include <inc/lexer.h>
#include <inc/dynarr.h>

/*
 * When isNan is true, val is undefined
 */
struct eval_result{
	bool isNan; // short cut case like:
		// defined __STDC_VERSION__ && __STDC_VERSION__ >= 199409L 
		// requires a NAN flag when __STDC_VERSION__ is not defined
		//
		// we have 3 short cut case right now: &&, ||, ?:
	int val;
};

static struct eval_result pp_unary_expr(struct lexer *lexer);
static struct eval_result pp_expr_internal(struct lexer *lexer, bool until_newline);

struct eval_result wrap_eval_result(int isNan, int val) {
	struct eval_result res;
	res.isNan = isNan;
	res.val = val;
	return res;
}

/*
 * XXX always take care to order the predecence when adding new operator
 */
static int pred_table[] = {
	[TOK_ADD] = 80,
	[TOK_SUB] = 80,

	[TOK_GT] = 50,
	[TOK_GE] = 50,
	[TOK_LT] = 50,
	[TOK_LE] = 50,

	[TOK_EQ] = 30,
	[TOK_NE] = 30,

	[TOK_LOGIC_AND] = 20,

	[TOK_LOGIC_OR] = 10,

	// right association
	[TOK_QUESTION] = 5,
	[TOK_COLON] = 5,

	[TOK_TOTAL_NUM] = 0,
};

static inline int pp_get_op_pred(unsigned int op) {
	int pred;
	if (op >= TOK_TOTAL_NUM || (pred = pred_table[op]) <= 0) {
		// assert(false); // TODO
		panic("invalid op %s", token_tag_str(op));
	}
	return pred;
}

static int pp_defined(struct lexer *lexer) {
	union token tok = lexer_next_token(lexer);

	if (tok.tok_tag == TOK_LPAREN) {
		tok = expect(lexer, TOK_IDENTIFIER);
		expect(lexer, TOK_RPAREN);
	}
	assume(tok, TOK_IDENTIFIER);
	bool result = macro_defined(lexer, tok.id.s);
	token_destroy(tok);
	return result;
}

static struct eval_result pp_eval_identifier(struct lexer *lexer, const char *name) {
	int old_no_expand_macro = lexer_push_config(lexer, no_expand_macro, 0);
	struct eval_result result;
	if (!try_expand_macro(lexer, name)) {
#if 0
		result = wrap_eval_result(true, 0); // return Nan
#else
		// this is what GCC do
		fprintf(stderr, "\033[31mpp_eval_identifier treat undefined id '%s' as 0\033[0m\n", name);
		result = wrap_eval_result(false, 0);
#endif
		goto out;
	}
	result = pp_unary_expr(lexer);
out:
	lexer_pop_config(lexer, no_expand_macro, old_no_expand_macro);
	return result;
}

static struct eval_result pp_unary_expr(struct lexer *lexer) {
	union token tok = lexer_next_token(lexer);
	struct eval_result result;

	switch (tok.tok_tag) {
	case TOK_EXCLAMATION:
		result = pp_unary_expr(lexer);
		result.val = !result.val;
		return result;
	case PP_TOK_DEFINED:
		result.val = pp_defined(lexer);
		result.isNan = false;
		return result;
	case TOK_IDENTIFIER:
		result = pp_eval_identifier(lexer, tok.id.s);
		token_destroy(tok);
		return result; 
	case TOK_CONSTANT_VALUE:
		if (!(tok.const_val.flags & CONST_VAL_TOK_INTEGER)) {
			panic("integer required");
		}
		result.val = tok.const_val.ival;
		result.isNan = false;
		return result;
	case TOK_SUB:
		result = pp_unary_expr(lexer);
		result.val = -result.val;
		return result;
	case TOK_LPAREN:
		return pp_expr_internal(lexer, false);
	default:
		file_reader_dump_remaining(lexer->cstream); 
		token_dump(tok); 
		panic("ni %s", token_tag_str(tok.tok_tag));
	}
}

static struct eval_result perform_conditional(struct eval_result cond, struct eval_result lhs, struct eval_result rhs) {
	struct eval_result invalid = wrap_eval_result(true, 0);
	if (cond.isNan) {
		return invalid;
	}
	if (cond.val) {
		return lhs;
	} else {
		return rhs;
	}
}

static struct eval_result perform_op(int op, struct eval_result lhs, struct eval_result rhs) {
	struct eval_result result = wrap_eval_result(true, 0);

	if (lhs.isNan) { // if lhs is invalid, the final result is always invalid
		return result;
	}

	// first have special handling for && and ||
	if (op == TOK_LOGIC_AND) {
		if (!lhs.val) {
			return wrap_eval_result(false, 0);
		}
		return rhs;
	}
	if (op == TOK_LOGIC_OR) {
		if (lhs.val) {
			return wrap_eval_result(false, 1);
		}
		return rhs;
	}

	// all other operator requires both operand to be valid
	if (rhs.isNan) { 
		return result;
	}

	int val = perform_int_bin_op(lhs.val, rhs.val, op);
	return wrap_eval_result(false, val);
}

static void drain_stk_below_pred(struct intstack *stk, struct intstack *nanstk, int waterline_pred) {
	assert(intstack_size(stk) > 0 && intstack_size(stk) % 2 == 1);
	assert(intstack_size(nanstk) * 2 - 1 == intstack_size(stk));

	struct eval_result v;
	v.val = intstack_pop(stk);
	v.isNan = intstack_pop(nanstk);

	while (intstack_size(stk) > 0 && pp_get_op_pred(intstack_top(stk)) >= waterline_pred) {
		int op = intstack_pop(stk);
		struct eval_result lhs;
		lhs.val = intstack_pop(stk);
		lhs.isNan = intstack_pop(nanstk);

		if (op == TOK_QUESTION) {
			panic("conditional without ':'");
		}
		if (op == TOK_COLON) {
			if (intstack_top(stk) == 0) {
				panic("conditional without '?'");
			}
			int llop = intstack_pop(stk);
			struct eval_result llhs;
			llhs.val = intstack_pop(stk);
			llhs.isNan = intstack_pop(nanstk);

			if (llop != TOK_QUESTION) {
				panic("expect '?'"); // NOTE: assume no operation with lower precedence than '?'/':'
			}
			v = perform_conditional(llhs, lhs, v);
		} else {
			v = perform_op(op, lhs, v);
		}
	}
	intstack_push(stk, v.val);
	intstack_push(nanstk, v.isNan);
}

static struct eval_result pp_expr_internal(struct lexer *lexer, bool until_newline) {
	union token tok;
	// nanstk is parallel to the operand part (exclude the operator part) of stk
	struct intstack *stk = intstack_init();
	struct intstack *nanstk = intstack_init();
	struct eval_result unary_res;

	unary_res = pp_unary_expr(lexer);
	intstack_push(stk, unary_res.val);
	intstack_push(nanstk, unary_res.isNan);

	tok = lexer_next_token(lexer);
	while (tok.tok_tag != TOK_NEWLINE && tok.tok_tag != TOK_RPAREN) {
		int pred = pp_get_op_pred(tok.tok_tag);
		unary_res = pp_unary_expr(lexer);

		// do the calculation
		drain_stk_below_pred(stk, nanstk, tok.tok_tag == TOK_QUESTION || tok.tok_tag == TOK_COLON ? pred + 1 : pred); // consider right association
		
		intstack_push(stk, tok.tok_tag);
		intstack_push(stk, unary_res.val);
		intstack_push(nanstk, unary_res.isNan);

		tok = lexer_next_token(lexer);
	}

	if (!until_newline && tok.tok_tag == TOK_NEWLINE) {
		panic("missing ')' in pp expression");
	}

	if (until_newline && tok.tok_tag == TOK_RPAREN) {
		panic("unmatching ')' in pp expression");
	}

	drain_stk_below_pred(stk, nanstk, 0);
	assert(intstack_size(stk) == 1);
	assert(intstack_size(nanstk) == 1);

	
	struct eval_result result;
	result.val = intstack_top(stk);
	result.isNan = intstack_top(nanstk);

	intstack_destroy(stk);
	intstack_destroy(nanstk);

	return result;
}

/*
 * The caller already set pp context
 */
int pp_expr(struct lexer *lexer, bool until_newline) {
	struct eval_result result = pp_expr_internal(lexer, until_newline);
	if (result.isNan) {
		file_reader_dump_remaining(lexer->cstream); // TODO
		panic("Invalid pp expression");
	}
	return result.val;
}

