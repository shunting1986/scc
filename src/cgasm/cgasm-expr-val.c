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
	struct expr_val ret;
	ret.type = EXPR_VAL_TEMP;
	ret.temp_var.ind = ctx->func_ctx->nlocal_word++;
	return ret;
}

struct expr_val const_expr_val(union token tok) {
	struct expr_val ret;
	ret.type = EXPR_VAL_CONST_VAL;
	ret.const_val = tok;
	return ret;
}

struct expr_val void_expr_val() {
	struct expr_val ret;
	ret.type = EXPR_VAL_VOID;
	return ret;
}

struct expr_val condcode_expr(int op, struct expr_val lhs, struct expr_val rhs, struct syntreebasenode *rhs_lazy) {
	struct expr_val ret;
	struct condcode *cc = mallocz(sizeof(*cc));
	ret.type = EXPR_VAL_CC;
	ret.cc = cc;
	
	cc->op = op;
	cc->lhs = lhs;
	cc->rhs = rhs;
	cc->rhs_lazy = rhs_lazy;
	return ret;
}

struct expr_val expr_val_add_deref_flag(struct expr_val val) {
	assert((val.type & EXPR_VAL_FLAG_DEREF) == 0);
	val.type |= EXPR_VAL_FLAG_DEREF;
	return val;
}


