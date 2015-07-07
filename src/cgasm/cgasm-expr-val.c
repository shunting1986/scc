#include <inc/cgasm.h>
#include <inc/util.h>

void load_val_to_reg(struct expr_val val, int reg) {
	panic("ni");
}

void store_reg_to_mem(int reg, struct expr_val mem) {
	panic("ni");
}

struct expr_val symbol_expr_val(struct symbol *sym) {
	struct expr_val ret;
	ret.type = EXPR_VAL_SYMBOL;
	ret.sym = sym;
	return ret;
}

struct expr_val str_literal_expr_val(int ind) {
	struct expr_val ret;
	ret.type = EXPR_VAL_STR_LITERAL;
	ret.ind = ind;
	return ret;
}

struct expr_val cgasm_alloc_temp_var(struct cgasm_context *ctx) {
	panic("ni");
}
