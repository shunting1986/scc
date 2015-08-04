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

static void cgasm_store_reg2_to_ll_sym(struct cgasm_context *ctx, int reg1, int reg2, struct symbol *sym) {
	int base_reg, offset;

	switch (sym->type) {
	case SYMBOL_LOCAL_VAR:
		base_reg = REG_EBP;
		offset = cgasm_get_local_var_offset(ctx, (struct local_var_symbol *) sym);
		break;
	default:
		panic("unsupported sym type %d", sym->type);
	}
	cgasm_println(ctx, "movl %%%s, %d(%%%s)", get_reg_str_code(reg1), offset, get_reg_str_code(base_reg));
	cgasm_println(ctx, "movl %%%s, %d(%%%s)", get_reg_str_code(reg2), offset + 4, get_reg_str_code(base_reg));
}

void cgasm_store_reg2_to_ll_mem(struct cgasm_context *ctx, int reg1, int reg2, struct expr_val mem) {
	switch (mem.type) {
	case EXPR_VAL_TEMP:
		cgasm_store_reg2_to_ll_temp(ctx, reg1, reg2, mem);
		break;
	case EXPR_VAL_SYMBOL:
		cgasm_store_reg2_to_ll_sym(ctx, reg1, reg2, mem.sym);
		break;
	default:
		panic("unsupported type %d", mem.type);
	}
}

static void cgasm_ll_neg(struct cgasm_context *ctx, struct expr_val diff, struct expr_val res) {
	assert(diff.ctype->tag == T_LONG_LONG);
	assert(res.ctype->tag == T_INT);

	int addr_reg = REG_ESI;
	cgasm_load_addr_to_reg(ctx, diff, addr_reg);

	int set0_label = cgasm_new_label_no(ctx);
	int out_label = cgasm_new_label_no(ctx);
	char buf[128];

	cgasm_println(ctx, "cmpl $0, 4(%%%s)", get_reg_str_code(addr_reg));
	cgasm_println(ctx, "jge %s", get_jump_label_str(set0_label, buf));
	cgasm_println(ctx, "movl $1, %s", cgasm_get_lval_asm_code(ctx, res, buf));
	cgasm_println(ctx, "jmp %s", get_jump_label_str(out_label, buf));
	cgasm_emit_jump_label(ctx, set0_label);
	cgasm_println(ctx, "movl $0, %s", cgasm_get_lval_asm_code(ctx, res, buf));
	cgasm_emit_jump_label(ctx, out_label);
}

static void cgasm_ll_pos(struct cgasm_context *ctx, struct expr_val diff, struct expr_val res) {
	assert(diff.ctype->tag == T_LONG_LONG);
	assert(res.ctype->tag == T_INT);

	int addr_reg = REG_ESI;
	cgasm_load_addr_to_reg(ctx, diff, addr_reg);

	int set0_label = cgasm_new_label_no(ctx);
	int set1_label = cgasm_new_label_no(ctx);
	int out_label = cgasm_new_label_no(ctx);
	char buf[128];

	cgasm_println(ctx, "cmpl $0, 4(%%%s)", get_reg_str_code(addr_reg));
	cgasm_println(ctx, "jl %s", get_jump_label_str(set0_label, buf));
	cgasm_println(ctx, "jg %s", get_jump_label_str(set1_label, buf));
	cgasm_println(ctx, "cmpl $0, (%%%s)", get_reg_str_code(addr_reg));
	cgasm_println(ctx, "je %s", get_jump_label_str(set0_label, buf));

	cgasm_emit_jump_label(ctx, set1_label);
	cgasm_println(ctx, "movl $1, %s", cgasm_get_lval_asm_code(ctx, res, buf));

	cgasm_println(ctx, "jmp %s", get_jump_label_str(out_label, buf));
	cgasm_emit_jump_label(ctx, set0_label);
	cgasm_println(ctx, "movl $0, %s", cgasm_get_lval_asm_code(ctx, res, buf));
	cgasm_emit_jump_label(ctx, out_label);
}

static struct expr_val cgasm_handle_binary_op_ll_cmp(struct cgasm_context *ctx, int op, struct expr_val lhs, struct expr_val rhs) {
	struct expr_val diff = cgasm_handle_binary_op_ll(ctx, TOK_SUB, lhs, rhs);
	struct expr_val res = cgasm_alloc_temp_var(ctx, get_int_type());
	switch (op) {
	case TOK_LT:
		cgasm_ll_neg(ctx, diff, res);
		break;
	case TOK_GT:
		cgasm_ll_pos(ctx, diff, res);
		break;
	default:
		panic("unsupported op %s", token_tag_str(op));
	}
	return res;
}

struct expr_val cgasm_handle_binary_op_ll(struct cgasm_context *ctx, int op, struct expr_val lhs, struct expr_val rhs) {
	assert(lhs.ctype != NULL);
	assert(rhs.ctype != NULL);
	assert(lhs.ctype->tag >= T_CHAR && lhs.ctype->tag <= T_LONG_LONG);
	assert(rhs.ctype->tag >= T_CHAR && rhs.ctype->tag <= T_LONG_LONG);
	assert(lhs.ctype->tag == T_LONG_LONG || rhs.ctype->tag == T_LONG_LONG);

	if (op == TOK_LSHIFT || op == TOK_RSHIFT) {
		cgasm_emit_abort(ctx);  // TODO not supported yet
		return ll_const_expr_val(0LL);
	}

	if (lhs.ctype->tag != T_LONG_LONG) {
		lhs = type_convert(ctx, lhs, get_long_long_type());
	}

	if (rhs.ctype->tag != T_LONG_LONG) {
		rhs = type_convert(ctx, rhs, get_long_long_type());
	}

	if (op == TOK_LT || op == TOK_GT) {
		return cgasm_handle_binary_op_ll_cmp(ctx, op, lhs, rhs);
	}

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
	case TOK_SUB:
		cgasm_println(ctx, "subl %%%s, %%%s", get_reg_str_code(rhs_reg1), get_reg_str_code(lhs_reg1));
		cgasm_println(ctx, "sbbl %%%s, %%%s", get_reg_str_code(rhs_reg2), get_reg_str_code(lhs_reg2));
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

struct expr_val cgasm_handle_ll_assign_op(struct cgasm_context *ctx, struct expr_val lhs, struct expr_val rhs, int op) {
	struct expr_val res;
	switch (op) {
	case TOK_ASSIGN:
		res = rhs;
		break;
	case TOK_ADD_ASSIGN:
		res = cgasm_handle_binary_op_ll(ctx, TOK_ADD, lhs, rhs);
		break;
	default:
		panic("unsupported op %s", token_tag_str(op));
	}

	int reg1 = REG_EAX, reg2 = REG_ECX;
	cgasm_load_ll_val_to_reg2(ctx, res, reg1, reg2);
	cgasm_store_reg2_to_ll_mem(ctx, reg1, reg2, lhs);
	return lhs;
}



