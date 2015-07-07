#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/lexer.h>

struct expr_val cgasm_handle_ampersand(struct cgasm_context *ctx, struct expr_val operand) {
	if (operand.type != EXPR_VAL_SYMBOL) {
		panic("'&' can only be applied to symbol");
	}
	operand.type = EXPR_VAL_SYMBOL_ADDR;
	return operand;
}

struct expr_val cgasm_handle_unary_op(struct cgasm_context *ctx, int tok_tag, struct expr_val operand) {
	switch (tok_tag) {
	case TOK_AMPERSAND:
		return cgasm_handle_ampersand(ctx, operand);
	default:
		panic("ni %s", token_tag_str(tok_tag));
	}
}
