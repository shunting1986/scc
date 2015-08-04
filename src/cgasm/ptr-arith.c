#include <inc/util.h>
#include <inc/cgasm-ptr-arith.h>
#include <inc/cgasm.h>

static struct expr_val cgasm_handle_ptr_cmp(struct cgasm_context *ctx, int tok_tag, struct expr_val lhs, struct expr_val rhs) {
	lhs = cgasm_handle_deref_flag(ctx, lhs);
	rhs = cgasm_handle_deref_flag(ctx, rhs);
	if (lhs.ctype->tag != T_PTR || rhs.ctype->tag != T_PTR) {
		panic("pointer required");
	}
	lhs.ctype = get_int_type(); // convert to int type
	rhs.ctype = get_int_type();
	return cgasm_handle_binary_op(ctx, tok_tag, lhs, rhs);
}

static struct expr_val cgasm_handle_ptr_add(struct cgasm_context *ctx, struct expr_val lhs, struct expr_val rhs) {
	lhs = cgasm_handle_deref_flag(ctx, lhs);
	rhs = cgasm_handle_deref_flag(ctx, rhs);
	assert(lhs.ctype->tag == T_PTR || rhs.ctype->tag == T_PTR);
	if (lhs.ctype->tag == T_PTR && rhs.ctype->tag == T_PTR) {
		panic("add ptr to ptr is not valid");
	}

	if (lhs.ctype->tag != T_PTR) {
		struct expr_val tmp = lhs;
		lhs = rhs;
		rhs = tmp;
	}

	if (!is_integer_type(rhs.ctype)) {
		panic("ptr is only allowed to add with integer type");
	}

	struct type *ptrtype = lhs.ctype;
	struct type *subtype = ptrtype->subtype;
	if (subtype->tag == T_VOID || type_get_size(subtype) == 1) {
		lhs.ctype = get_int_type(); // convert to int
		struct expr_val res = cgasm_handle_binary_op(ctx, TOK_ADD, lhs, rhs);
		res = cgasm_handle_deref_flag(ctx, res);
		res.ctype = ptrtype;
		return res;
	}
	panic("ni");
}

struct expr_val cgasm_handle_ptr_binary_op(struct cgasm_context *ctx, int tok_tag, struct expr_val lhs, struct expr_val rhs) {
	switch (tok_tag) {
	case TOK_EQ: case TOK_NE: case TOK_GT: case TOK_LT: case TOK_GE: case TOK_LE:
		return cgasm_handle_ptr_cmp(ctx, tok_tag, lhs, rhs);	
	case TOK_ADD:
		return cgasm_handle_ptr_add(ctx, lhs, rhs);
	default:
		panic("ni %s", token_tag_str(tok_tag));
	}
}

