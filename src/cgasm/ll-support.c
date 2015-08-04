#include <inc/ll-support.h>
#include <inc/util.h>
#include <inc/symtab.h>
#include <inc/cgasm.h>

// TODO use string operation as a optimization
static void cgasm_push_bytes(struct cgasm_context *ctx, int from_base_reg, int from_start_off, int size) {
	int end = from_start_off + (size + 3) / 4 * 4;
	int off;
	int reg = REG_EAX;

	for (off = end - 4; off >= from_start_off; off -= 4) {
		cgasm_println(ctx, "movl %d(%%%s), %%%s", off, get_reg_str_code(from_base_reg), get_reg_str_code(reg));
		cgasm_println(ctx, "pushl %%%s", get_reg_str_code(reg));
	}
}

void cgasm_push_ll_val(struct cgasm_context *ctx, struct expr_val val) {
	struct type *type = expr_val_get_type(val);
	assert(type->tag == T_LONG_LONG);

	// int base_reg = REG_EAX; // cgasm_push_bytes already use EAX
	int base_reg = REG_ESI;
	cgasm_load_addr_to_reg(ctx, val, base_reg);
	cgasm_push_bytes(ctx, base_reg, 0, 8);
}

// XXX as a simple solution, we use RSI as temp register. A better solution is pass-in the reg constaints
// The constaints is imposed by cgasm_handle_binary_op_ll
void cgasm_load_ll_val_to_reg2(struct cgasm_context *ctx, struct expr_val val, int reg1, int reg2) {
	// const value is different
	if (val.type == EXPR_VAL_CONST_VAL) {
		int flags = val.const_val.const_val.flags;
		assert(flags & CONST_VAL_TOK_LONG_LONG);
		long long llv = val.const_val.const_val.llval;
		cgasm_println(ctx, "movl $%d, %%%s", (int) (llv & 0xFFFFFFFF), get_reg_str_code(reg1));
		cgasm_println(ctx, "movl $%d, %%%s", (int) ((llv >> 32)& 0xFFFFFFFF), get_reg_str_code(reg2));
		return;
	}

	int addr_reg = REG_ESI;
	cgasm_load_addr_to_reg(ctx, val, addr_reg);
	cgasm_println(ctx, "movl (%%%s), %%%s", get_reg_str_code(addr_reg), get_reg_str_code(reg1));
	cgasm_println(ctx, "movl 4(%%%s), %%%s", get_reg_str_code(addr_reg), get_reg_str_code(reg2));
}

void cgasm_store_reg2_to_ll_temp(struct cgasm_context *ctx, int reg1, int reg2, struct expr_val temp) {
	assert(temp.type == EXPR_VAL_TEMP);
	int offset = cgasm_get_temp_var_offset(ctx, temp.temp_var);
	cgasm_println(ctx, "movl %%%s, %d(%%ebp)", get_reg_str_code(reg1), offset);
	cgasm_println(ctx, "movl %%%s, %d(%%ebp)", get_reg_str_code(reg2), offset + 4);
}


struct expr_val cgasm_handle_binary_op_ll(struct cgasm_context *ctx, int op, struct expr_val lhs, struct expr_val rhs) {
	assert(lhs.ctype != NULL && lhs.ctype->tag == T_LONG_LONG);
	if (op == TOK_LSHIFT || op == TOK_RSHIFT) {
		cgasm_emit_abort(ctx);  // TODO not supported yet
		return ll_const_expr_val(0LL);
	}

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
		cgasm_println(ctx, "adcl %%%s, %%%s", get_reg_str_code(rhs_reg2), get_reg_str_code(lhs_reg2));
		cgasm_store_reg2_to_ll_temp(ctx, lhs_reg1, lhs_reg2, ret);
		break;
	case TOK_AMPERSAND:
		cgasm_println(ctx, "andl %%%s, %%%s", get_reg_str_code(rhs_reg1), get_reg_str_code(lhs_reg1));
		cgasm_println(ctx, "andl %%%s, %%%s", get_reg_str_code(rhs_reg2), get_reg_str_code(lhs_reg2));
		cgasm_store_reg2_to_ll_temp(ctx, lhs_reg1, lhs_reg2, ret);
		break;
	case TOK_VERT_BAR:
		cgasm_println(ctx, "orl %%%s, %%%s", get_reg_str_code(rhs_reg1), get_reg_str_code(lhs_reg1));
		cgasm_println(ctx, "orl %%%s, %%%s", get_reg_str_code(rhs_reg2), get_reg_str_code(lhs_reg2));
		cgasm_store_reg2_to_ll_temp(ctx, lhs_reg1, lhs_reg2, ret);
		break;
	default:
		panic("ni %s", token_tag_str(op));
	}

	return ret;
}


