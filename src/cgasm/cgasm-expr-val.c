#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/symtab.h>

static const char *reg_name_list[] = {
	"eax", "ecx", "edx", "ebx",
	"esi", "edi", "esp", "ebp",
};

const char *get_reg_str_code(unsigned int reg) {
	assert(reg < REG_NUM);
	return reg_name_list[reg];
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


