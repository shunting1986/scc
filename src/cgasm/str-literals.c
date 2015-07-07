#include <inc/cgasm.h>

void cgasm_destroy_str_literals(struct cgasm_context *ctx) {
	dynarr_destroy(ctx->str_literals); // NOTE: the storage for string is released
	  // when destroying the syntax tree
}

struct expr_val cgasm_register_str_literal(struct cgasm_context *ctx, char *str) {
	int ind = dynarr_size(ctx->str_literals);
	dynarr_add(ctx->str_literals, str);
	return str_literal_expr_val(ind);
}
