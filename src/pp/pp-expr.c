#include <inc/pp.h>
#include <inc/util.h>
#include <inc/lexer.h>
#include <inc/dynarr.h>

static int pp_unary_expr(struct lexer *lexer);

/*
 * XXX always take care to order the predecence when adding new operator
 */
static int pred_table[] = {
	[TOK_LOGIC_AND] = 1,
	[TOK_GT] = 2,

	[TOK_TOTAL_NUM] = 0,
};

static inline int pp_get_op_pred(unsigned int op) {
	int pred;
	if (op >= TOK_TOTAL_NUM || (pred = pred_table[op]) <= 0) {
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

static int pp_eval_identifier(struct lexer *lexer, const char *name) {
	int old_no_expand_macro = lexer_push_config(lexer, no_expand_macro, 0);
	if (!try_expand_macro(lexer, name)) {
		panic("invalid identifier here: %s", name);
	}
	lexer_pop_config(lexer, no_expand_macro, old_no_expand_macro);
	return pp_unary_expr(lexer);
}

static int pp_unary_expr(struct lexer *lexer) {
	union token tok = lexer_next_token(lexer);
	int ret;
	switch (tok.tok_tag) {
	case TOK_EXCLAMATION:
		return !pp_unary_expr(lexer);
	case PP_TOK_DEFINED:
		return pp_defined(lexer);
	case TOK_IDENTIFIER:
		ret = pp_eval_identifier(lexer, tok.id.s);
		token_destroy(tok);
		return ret;
	case TOK_CONSTANT_VALUE:
		if (!(tok.const_val.flags & CONST_VAL_TOK_INTEGER)) {
			panic("integer required");
		}
		return tok.const_val.ival;
	case TOK_SUB:
		return -pp_unary_expr(lexer);
	default:
		token_dump(tok); 
		panic("ni %s", token_tag_str(tok.tok_tag));
	}
}

static int perform_op(int op, int lhs, int rhs) {
	switch (op) {
	case TOK_LOGIC_AND:
		return lhs && rhs;
	case TOK_GT:
		return lhs > rhs;
	default:
		panic("unsupported op %s", token_tag_str(op));
	}
}

static void drain_stk_below_pred(struct intstack *stk, int waterline_pred) {
	assert(intstack_size(stk) > 0 && intstack_size(stk) % 2 == 1);
	int v = intstack_pop(stk);

	while (intstack_size(stk) > 0 && pp_get_op_pred(intstack_top(stk)) >= waterline_pred) {
		int op = intstack_pop(stk);
		int lhs = intstack_pop(stk);
		v = perform_op(op, lhs, v);
	}
	intstack_push(stk, v);
}

/*
 * The caller already set pp context
 */
int pp_expr(struct lexer *lexer) {
	union token tok;
	struct intstack *stk = intstack_init();

	intstack_push(stk, pp_unary_expr(lexer));

	tok = lexer_next_token(lexer);
	while (tok.tok_tag != TOK_NEWLINE) {
		int pred = pp_get_op_pred(tok.tok_tag);
		int v = pp_unary_expr(lexer);

		// do the calculation
		drain_stk_below_pred(stk, pred);
		
		intstack_push(stk, tok.tok_tag);
		intstack_push(stk, v);

		tok = lexer_next_token(lexer);
	}

	drain_stk_below_pred(stk, 0);
	assert(intstack_size(stk) == 1);
	int result = intstack_top(stk);
	intstack_destroy(stk);

	fprintf(stderr, "pp_expr returns %d\n", result);
	return result;
}
