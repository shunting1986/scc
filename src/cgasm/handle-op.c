#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/lexer.h>
#include <inc/symtab.h>

/**
 * handle unary op, binary op, push op, load op, store op etc.
 */

/**********************/
/* util               */
/**********************/
static int get_local_var_offset(struct local_var_symbol *sym) {
	return -(sym->var_ind * 4 + 4);
}

static int get_temp_var_offset(struct temp_var temp) {
	return -(temp.ind * 4 + 4);
}

static void get_lval_local_var_asm_code(struct local_var_symbol *sym, char *buf) {
	assert(buf != NULL);
	sprintf(buf, "%d(%%esp)", get_local_var_offset(sym));
}

static void get_lval_sym_asm_code(struct symbol *sym, char *buf) {
	assert(buf != NULL);
	switch (sym->type) {
	case SYMBOL_LOCAL_VAR:
		get_lval_local_var_asm_code((struct local_var_symbol *) sym, buf);
		break;
	default:
		panic("ni %d", sym->type);
		break;
	}
}

static char *get_lval_asm_code(struct expr_val val, char *buf) {
	if (buf == NULL) {
		buf = malloc(128); // caller should free it
	}
	switch (val.type) {
	case EXPR_VAL_SYMBOL:
		get_lval_sym_asm_code(val.sym, buf);
		break;
	default:
		panic("ni %d", val.type);
	}
	return buf;
}

/**********************/
/* load               */
/**********************/
static void cgasm_load_local_var_to_reg(struct cgasm_context *ctx, struct local_var_symbol *sym, int reg) {
	cgasm_println(ctx, "movl %%%s, %d(%%esp)", get_reg_str_code(reg), get_local_var_offset(sym));
}

static void cgasm_load_sym_to_reg(struct cgasm_context *ctx, struct symbol *sym, int reg) {
	switch (sym->type) {
	case SYMBOL_LOCAL_VAR:
		cgasm_load_local_var_to_reg(ctx, (struct local_var_symbol *) sym, reg);
		break;
	default:
		panic("ni %d %s", sym->type, sym->name);
	}
}

static void cgasm_load_temp_to_reg(struct cgasm_context *ctx, struct temp_var temp, int reg) {
	cgasm_println(ctx, "movl %%%s, %d(%%esp)", get_reg_str_code(reg), get_temp_var_offset(temp));
}

static void cgasm_load_const_val_to_reg(struct cgasm_context *ctx, union token const_val, int reg) {
	int flags = const_val.const_val.flags;
	if (flags & CONST_VAL_TOK_INTEGER) {
		cgasm_println(ctx, "movl %%%s, $%d", get_reg_str_code(reg), const_val.const_val.ival);
	} else {
		panic("ni");
	}
}

void cgasm_load_val_to_reg(struct cgasm_context *ctx, struct expr_val val, int reg) {
	switch (val.type) {
	case EXPR_VAL_SYMBOL:
		cgasm_load_sym_to_reg(ctx, val.sym, reg);
		break;
	case EXPR_VAL_TEMP:
		cgasm_load_temp_to_reg(ctx, val.temp_var, reg);
		break;
	case EXPR_VAL_CONST_VAL:
		cgasm_load_const_val_to_reg(ctx, val.const_val, reg);
		break;
	default:
		panic("ni %d", val.type);
		break;
	}
}

/***********************/
/* store               */
/***********************/
static void cgasm_store_reg_to_temp_var(struct cgasm_context *ctx, int reg, struct temp_var temp) {
	cgasm_println(ctx, "movl %d(%%esp), %%%s", get_temp_var_offset(temp), get_reg_str_code(reg));
}

void cgasm_store_reg_to_mem(struct cgasm_context *ctx, int reg, struct expr_val mem) {
	switch (mem.type) {
	case EXPR_VAL_TEMP:
		cgasm_store_reg_to_temp_var(ctx, reg, mem.temp_var);
		break;
	default:
		panic("ni %d", mem.type);
	}
}

/***********************/
/* push                */
/***********************/
static void cgasm_push_str_literal(struct cgasm_context *ctx, int ind) {
	char buf[256];
	get_str_literal_label(ind, buf);
	cgasm_println(ctx, "pushl $%s", buf);
}

static void cgasm_push_local_var_addr(struct cgasm_context *ctx, struct local_var_symbol *sym) {
	cgasm_println(ctx, "leal %%eax, %d(%%esp)", get_local_var_offset(sym));
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

static void cgasm_push_sym(struct cgasm_context *ctx, struct symbol *sym) {
	// load the register first
	int reg = REG_EAX;
	cgasm_load_sym_to_reg(ctx, sym, reg);
	cgasm_println(ctx, "pushl %%%s", get_reg_str_code(reg));
}

void cgasm_push_val(struct cgasm_context *ctx, struct expr_val val) {
	switch (val.type) {
	case EXPR_VAL_SYMBOL_ADDR:
		cgasm_push_sym_addr(ctx, val.sym);
		break;
	case EXPR_VAL_STR_LITERAL:
		cgasm_push_str_literal(ctx, val.ind);
		break;
	case EXPR_VAL_SYMBOL:
		cgasm_push_sym(ctx, val.sym);
		break;
	default:
		panic("ni %d", val.type);
	}
}

/***********************/
/* unary op            */
/***********************/
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

/***********************/
/* binary op           */
/***********************/
struct expr_val cgasm_handle_binary_op(struct cgasm_context *ctx, int tok_tag, struct expr_val lhs, struct expr_val rhs) {
	int lhs_reg = REG_EAX;
	int rhs_reg = REG_ECX;
	cgasm_load_val_to_reg(ctx, lhs, lhs_reg);
	cgasm_load_val_to_reg(ctx, rhs, rhs_reg);
	struct expr_val res;

	switch (tok_tag) {
	case TOK_ADD:
		cgasm_println(ctx, "addl %%%s, %%%s", get_reg_str_code(lhs_reg), get_reg_str_code(rhs_reg));
		break;
	default:
		panic("ni %s", token_tag_str(tok_tag));
	}

	res = cgasm_alloc_temp_var(ctx);
	cgasm_store_reg_to_mem(ctx, lhs_reg, res);
	return res;
}

/*******************************/
/* assignment                  */
/*******************************/
struct expr_val cgasm_handle_assign_op(struct cgasm_context *ctx, struct expr_val lhs, struct expr_val rhs, int op) {
	int rhs_reg = REG_EAX;
	char buf[128];
	cgasm_load_val_to_reg(ctx, rhs, rhs_reg);

	switch (op) {
	case TOK_ASSIGN:
		cgasm_println(ctx, "movl %s, %s", get_lval_asm_code(lhs, buf), get_reg_str_code(rhs_reg));
		break;
	default:
		panic("ni %s", token_tag_str(op));
	}
	return lhs;
}

/********************************/
/* return op                    */
void cgasm_handle_ret(struct cgasm_context *ctx) {
	panic("ni");
	cgasm_println(ctx, "ret");
}
