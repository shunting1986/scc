#include <inc/cgasm.h>
#include <inc/util.h>

struct expr_val symbol_expr_val(struct symbol *sym) {
	panic("ni");
}

struct expr_val str_literal_expr_val(int ind) {
	struct expr_val ret;
	ret.type = EXPR_VAL_STR_LITERAL;
	ret.ind = ind;
	return ret;
}
