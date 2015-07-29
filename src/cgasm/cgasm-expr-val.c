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

/*
 * Rewrite enumerator to constant value
 */
struct expr_val symbol_expr_val(struct symbol *sym) {
	if (sym->type == SYMBOL_ENUMERATOR) {
		return int_const_expr_val(((struct enumerator_symbol *) sym)->val);
	}

	struct expr_val ret;
	ret.type = EXPR_VAL_SYMBOL;
	ret.sym = sym;

	// set expr_val type
	ret.ctype = sym->ctype;
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

struct expr_val int_const_expr_val(int val) {
	return const_expr_val(wrap_int_const_to_token(val));
}

struct expr_val const_expr_val(union token tok) {
	struct expr_val ret;
	ret.type = EXPR_VAL_CONST_VAL;
	ret.const_val = tok;
	return ret;
}

bool is_int_const(struct expr_val val) {
	return val.type == EXPR_VAL_CONST_VAL && (val.const_val.const_val.flags & CONST_VAL_TOK_INTEGER);
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

int expr_val_has_deref_flag(struct expr_val val) {
	return (val.type & EXPR_VAL_FLAG_DEREF) != 0;
}

struct expr_val expr_val_add_deref_flag(struct expr_val val) {
	assert((val.type & EXPR_VAL_FLAG_DEREF) == 0);
	assert(val.ctype->tag == T_PTR);
	val.type |= EXPR_VAL_FLAG_DEREF;
	return val;
}

struct expr_val expr_val_remove_deref_flag(struct expr_val val) {
	assert((val.type & EXPR_VAL_FLAG_DEREF) != 0);
	assert(val.ctype->tag == T_PTR);
	val.type &= ~EXPR_VAL_FLAG_DEREF;
	return val;
}

int expr_val_get_elem_size(struct expr_val val) {
	struct type *expr_type = expr_val_get_type(val);
	struct type *elem_type = type_get_elem_type(expr_type);
	return type_get_size(elem_type);
}

/*
 * handle deref flag here
 */
struct type *expr_val_get_type(struct expr_val val) {
	struct type *type = val.ctype;
	assert(type != NULL);

	if (val.type & EXPR_VAL_FLAG_DEREF) {
		type = type_deref(type);
	}
	return type;
}

