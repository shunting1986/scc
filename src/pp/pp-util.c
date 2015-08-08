#include <inc/pp.h>
#include <inc/util.h>
#include <inc/lexer.h>

// TODO use the macro
int push_want_quotation(struct lexer *lexer, int newval) {
	int oldval = lexer->want_quotation;
	lexer->want_quotation = newval;
	return oldval;
}

// TODO use the macro
void pop_want_quotation(struct lexer *lexer, int oldval) {
	lexer->want_quotation = oldval;
}

// TODO use the macro
int push_want_newline(struct lexer *lexer, int newval) {
	int oldval = lexer->want_newline;
	lexer->want_newline = newval;
	return oldval;
}

// TODO use the macro
void pop_want_newline(struct lexer *lexer, int oldval) {
	lexer->want_newline = oldval;
}

int perform_int_bin_op(int lhs, int rhs, int op) {
	int val;
	switch (op) {
	case TOK_GT: val = lhs > rhs; break;
	case TOK_LT: val = lhs < rhs; break;
	case TOK_LE: val = lhs <= rhs; break;
	case TOK_GE: val = lhs >= rhs; break;
	case TOK_EQ: val = lhs == rhs; break;
	case TOK_NE: val = lhs != rhs; break;
	case TOK_SUB: val = lhs - rhs; break;
	case TOK_ADD: val = lhs + rhs; break;
	case TOK_VERT_BAR: val = lhs | rhs; break;
	case TOK_STAR: val = lhs * rhs; break;
	case TOK_LSHIFT: val = lhs << rhs; break;
	case TOK_RSHIFT: val = lhs >> rhs; break;
	case TOK_DIV:
		if (rhs == 0) {
			panic("div 0");
		}
		val = lhs / rhs;
		break;
	default:
		panic("unsupported op %s", token_tag_str(op));
	}
	return val;
}
