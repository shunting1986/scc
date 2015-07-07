#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/lexer.h>
#include <inc/symtab.h>

/**
 * handle unary op, binary op, push op etc.
 */

static void cgasm_push_str_literal(struct cgasm_context *ctx, int ind) {
	char buf[256];
	get_str_literal_label(ind, buf);
	cgasm_println(ctx, "pushl $%s", buf);
}

static void cgasm_push_local_var_addr(struct cgasm_context *ctx, struct local_var_symbol *sym) {
	int offset = sym->var_ind * 4 + 4;
	cgasm_println(ctx, "leal %%eax, -%d(%%esp)", offset);
	cgasm_println(ctx, "pushl %%eax");
}

static void cgasm_push_sym_addr(struct cgasm_context *ctx, struct symbol *sym) {
	switch (sym->type) {
	case SYMBOL_LOCAL_VAR:
		cgasm_push_local_var_addr(ctx, (struct local_var_symbol *) sym);
		break;
	default:
		panic("ni %d", sym->type);
	}
}

void cgasm_push_val(struct cgasm_context *ctx, struct expr_val val) {
	switch (val.type) {
	case EXPR_VAL_SYMBOL_ADDR:
		cgasm_push_sym_addr(ctx, val.sym);
		break;
	case EXPR_VAL_STR_LITERAL:
		cgasm_push_str_literal(ctx, val.ind);
		break;
	default:
		panic("ni %d", val.type);
	}
}

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
