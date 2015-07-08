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
	return sym->var_ind * 4 + 4;
}

/**********************/
/* load               */
/**********************/
static void cgasm_load_local_var_to_reg(struct cgasm_context *ctx, struct local_var_symbol *sym, int reg) {
	cgasm_println(ctx, "movl %%%s -%d(%%esp)", get_reg_str_code(reg), get_local_var_offset(sym));
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

void cgasm_load_val_to_reg(struct cgasm_context *ctx, struct expr_val val, int reg) {
	switch (val.type) {
	case EXPR_VAL_SYMBOL:
		cgasm_load_sym_to_reg(ctx, val.sym, reg);
		break;
	default:
		panic("ni %d", val.type);
		break;
	}
}

/***********************/
/* store               */
/***********************/
void cgasm_store_reg_to_mem(struct cgasm_context *ctx, int reg, struct expr_val mem) {
	panic("ni");
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
	cgasm_println(ctx, "leal %%eax, -%d(%%esp)", get_local_var_offset(sym));
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
		cgasm_println(ctx, "addl %%%s %%%s", get_reg_str_code(lhs_reg), get_reg_str_code(rhs_reg));
		break;
	default:
		panic("ni %s", token_tag_str(tok_tag));
	}

	res = cgasm_alloc_temp_var(ctx);
	cgasm_store_reg_to_mem(ctx, lhs_reg, res);
	return res;
}


