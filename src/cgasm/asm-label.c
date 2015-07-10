#include <inc/cgasm.h>
#include <inc/util.h>

/**********************/
/* string literal     */
/**********************/
#define LABEL_FMT "STRING_LITERAL_%d"

char *get_str_literal_label(int ind, char *buf) {
	if (buf == NULL) {
		buf = mallocz(256); // NEED FREE BY CALLER
	}
	sprintf(buf, LABEL_FMT, ind);
	return buf;
}

void cgasm_destroy_str_literals(struct cgasm_context *ctx) {
	dynarr_destroy(ctx->str_literals); // NOTE: the storage for string is released
	  // when destroying the syntax tree
}

struct expr_val cgasm_register_str_literal(struct cgasm_context *ctx, char *str) {
	int ind = dynarr_size(ctx->str_literals);
	dynarr_add(ctx->str_literals, str);
	return str_literal_expr_val(ind);
}

void cgasm_dump_string_literals(struct cgasm_context *ctx) {
	char buf[256];
	int i;
	for (i = 0; i < dynarr_size(ctx->str_literals); i++) {
		char *s = dynarr_get(ctx->str_literals, i);

		// dump one string literal
		cgasm_println_noind(ctx, "%s:", get_str_literal_label(i, buf));
		cgasm_println(ctx, ".string \"%s\"", s);
	}
}

/**********************/
/* jump label         */
/**********************/
int cgasm_new_label_no(struct cgasm_context *ctx) {
	panic("ni");
}

void cgasm_emit_jump_label(struct cgasm_context *ctx, int no) {
	panic("ni");
}

char *get_jump_label_str(int no, char *buf) {
	panic("ni");
}

