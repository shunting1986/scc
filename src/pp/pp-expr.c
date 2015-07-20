#include <inc/pp.h>
#include <inc/util.h>
#include <inc/lexer.h>
#include <inc/dynarr.h>

/*
 * XXX always take care to order the predecence when adding new operator
 */
static int pred_table[] = {
	[TOK_LOGIC_AND] = 1,

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
	assume(tok, TOK_IDENTIFIER); // TODO handle parenthesis
	bool result = macro_defined(lexer, tok.id.s);
	token_destroy(tok);
	return result;
}

static int pp_unary_expr(struct lexer *lexer) {
	union token tok = lexer_next_token(lexer);
	switch (tok.tok_tag) {
	case TOK_EXCLAMATION:
		return !pp_unary_expr(lexer);
	case PP_TOK_DEFINED:
		return pp_defined(lexer);
	default:
		panic("ni %s", token_tag_str(tok.tok_tag));
	}
}

static int perform_op(int op, int lhs, int rhs) {
	switch (op) {
	case TOK_LOGIC_AND:
		return lhs && rhs;
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
