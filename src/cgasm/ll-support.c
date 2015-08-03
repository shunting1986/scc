#include <inc/ll-support.h>
#include <inc/util.h>
#include <inc/symtab.h>
#include <inc/cgasm.h>

// TODO use string operation as a optimization
static void cgasm_push_bytes(struct cgasm_context *ctx, int from_base_reg, int from_start_off, int size) {
	int end = from_start_off + size;
	int off;
	int reg = REG_EAX;
	for (off = from_start_off; off < end; off += 4) {
		cgasm_println(ctx, "movl %d(%%%s), %%%s", off, get_reg_str_code(from_base_reg), get_reg_str_code(reg));
		cgasm_println(ctx, "pushl %%%s", get_reg_str_code(reg));
	}
}

static void cgasm_load_ll_sym_to_reg2(struct cgasm_context *ctx, struct symbol *sym, int reg1, int reg2) {
	int offset;
	switch (sym->type) {
	case SYMBOL_LOCAL_VAR:
		offset = cgasm_get_local_var_offset(ctx, (struct local_var_symbol *) sym);
		cgasm_push_bytes(ctx, REG_EBP, offset, 8);
		break;
	default:	
		panic("ni %d", sym->type);
	}
}

// XXX as a simple solution, we use RSI as temp register. A better solution is pass-in the reg constaints
void cgasm_load_ll_val_to_reg2(struct cgasm_context *ctx, struct expr_val val, int reg1, int reg2) {
	int addr_reg = REG_ESI;
	cgasm_load_addr_to_reg(ctx, val, addr_reg);
	cgasm_println(ctx, "movl (%%%s), %%%s", get_reg_str_code(addr_reg), get_reg_str_code(reg1));
	cgasm_println(ctx, "movl (%%%s), %%%s", get_reg_str_code(addr_reg), get_reg_str_code(reg2));
}

static void cgasm_store_reg2_to_ll_temp(struct cgasm_context *ctx, int reg1, int reg2, struct expr_val temp) {
	panic("ni");
}

void cgasm_push_ll_sym(struct cgasm_context *ctx, struct symbol *sym) {
	struct type *type = sym->ctype;
	assert(type->tag == T_LONG_LONG);
	int reg1 = REG_EAX, reg2 = REG_EDX;
	cgasm_load_ll_sym_to_reg2(ctx, sym, reg1, reg2);

	cgasm_println(ctx, "pushl %%%s", get_reg_str_code(reg2));	
	cgasm_println(ctx, "pushl %%%s", get_reg_str_code(reg1));
}


struct expr_val cgasm_handle_binary_op_ll(struct cgasm_context *ctx, int op, struct expr_val lhs, struct expr_val rhs) {
	assert(lhs.ctype != NULL && lhs.ctype->tag == T_LONG_LONG);
	assert(rhs.ctype != NULL && rhs.ctype->tag == T_LONG_LONG); // TODO: should be able to handle long long + int etc.

	int lhs_reg1 = REG_EAX;
	int lhs_reg2 = REG_ECX;
	int rhs_reg1 = REG_EDX;
	int rhs_reg2 = REG_EBX;

	struct expr_val ret = cgasm_alloc_temp_var(ctx, get_long_long_type());

	cgasm_load_ll_val_to_reg2(ctx, lhs, lhs_reg1, lhs_reg2);
	cgasm_load_ll_val_to_reg2(ctx, rhs, rhs_reg1, rhs_reg2);

	switch (op) {
	case TOK_ADD:
		cgasm_println(ctx, "addl %%%s, %%%s", get_reg_str_code(rhs_reg1), get_reg_str_code(lhs_reg1));
		cgasm_println(ctx, "adcl %%%s, %%%s", get_reg_str_code(rhs_reg1), get_reg_str_code(lhs_reg1));
		cgasm_store_reg2_to_ll_temp(ctx, lhs_reg1, lhs_reg2, ret);
		break;
	default:
		panic("ni %s", token_tag_str(op));
	}

	return ret;
}


