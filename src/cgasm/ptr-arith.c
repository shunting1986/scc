#include <inc/util.h>
#include <inc/cgasm-ptr-arith.h>
#include <inc/cgasm.h>

static struct expr_val cgasm_handle_ptr_eq(struct cgasm_context *ctx, int tok_tag, struct expr_val lhs, struct expr_val rhs) {
	lhs = cgasm_handle_deref_flag(ctx, lhs);
	rhs = cgasm_handle_deref_flag(ctx, rhs);
	if (lhs.ctype->tag != T_PTR || rhs.ctype->tag != T_PTR) {
		panic("pointer required");
	}
	lhs.ctype = get_int_type(); // convert to int type
	rhs.ctype = get_int_type();
	return cgasm_handle_binary_op(ctx, tok_tag, lhs, rhs);
}

struct expr_val cgasm_handle_ptr_binary_op(struct cgasm_context *ctx, int tok_tag, struct expr_val lhs, struct expr_val rhs) {
	switch (tok_tag) {
	case TOK_EQ: case TOK_NE:
		return cgasm_handle_ptr_eq(ctx, tok_tag, lhs, rhs);	
	default:
		panic("ni %s", token_tag_str(tok_tag));
	}
}

