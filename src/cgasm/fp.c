#include <inc/util.h>
#include <inc/fp.h>
#include <inc/type.h>
#include <inc/cgasm.h>
#include <inc/token.h>

static void load_int_to_fpstk(struct cgasm_context *ctx, struct expr_val val);
static void load_ll_to_fpstk(struct cgasm_context *ctx, struct expr_val val);

void cgasm_push_fp_val_to_gstk(struct cgasm_context *ctx, struct expr_val val) {
	struct type *type = expr_val_get_type(val);
	int addr_reg = REG_EAX;
	assert(is_floating_type(type));
	cgasm_load_addr_to_reg(ctx, val, addr_reg);
	cgasm_push_bytes(ctx, addr_reg, 0, type_get_size(type));
}

static struct expr_val load_double_const_to_temp(struct cgasm_context *ctx, double v) {
	struct expr_val temp = cgasm_alloc_temp_var(ctx, get_double_type());
	union {
		struct {
			int low;
			int hei;
		} pair;
		double v;
	} conv;
	conv.v = v; 

	int addr_reg = REG_EAX;
	cgasm_load_addr_to_reg(ctx, temp, addr_reg);
	cgasm_println(ctx, "movl $%d, (%%%s)", conv.pair.low, get_reg_str_code(addr_reg));
	cgasm_println(ctx, "movl $%d, 4(%%%s)", conv.pair.hei, get_reg_str_code(addr_reg));
	return temp;
}

static void load_val_to_fpstk(struct cgasm_context *ctx, struct expr_val val) {
	struct type *type = expr_val_get_type(val);
	int addr_reg = REG_EAX;

	if (val.type == EXPR_VAL_CONST_VAL) {
		double double_val = const_token_to_double(val.const_val);
		struct expr_val temp = load_double_const_to_temp(ctx, double_val);
		load_val_to_fpstk(ctx, temp);
		return;
	}

	switch (type->tag) {
	case T_DOUBLE:
		// XXX this can be optimized to avoiding using the extra reg
		cgasm_load_addr_to_reg(ctx, val, addr_reg);
		cgasm_println(ctx, "fldl (%%%s)", get_reg_str_code(addr_reg));
		break;
	case T_INT:
		load_int_to_fpstk(ctx, val);
		break;
	case T_LONG_LONG:
		load_ll_to_fpstk(ctx, val);
		break;
	default:
		panic("type %d", type->tag);
		break;
	}
}

static void load_ll_to_fpstk(struct cgasm_context *ctx, struct expr_val val) {
	assert(expr_val_get_type(val)->tag == T_LONG_LONG);
	int reg = REG_EAX;
	cgasm_load_addr_to_reg(ctx, val, reg);
	cgasm_println(ctx, "fildq (%%%s)", get_reg_str_code(reg));
}

static void load_int_to_fpstk(struct cgasm_context *ctx, struct expr_val val) {
	assert(expr_val_get_type(val)->tag == T_INT);
	int reg = REG_EAX;
	cgasm_load_addr_to_reg(ctx, val, reg);
	cgasm_println(ctx, "fildl (%%%s)", get_reg_str_code(reg));
}

// If dst is integer type, this method will pop the double value, convert it to integer
// and then stored to dst.
static void pop_fpstk_to_lval(struct cgasm_context *ctx, struct expr_val dst) {
	struct type *type = expr_val_get_type(dst);
	char buf[256];
	int mask = 0;
	cgasm_get_lval_asm_code(ctx, dst, buf, &mask);
	if (type->tag == T_FLOAT) {
		cgasm_println(ctx, "fstps %s", buf);
	} else if (type->tag == T_DOUBLE) {
		cgasm_println(ctx, "fstpl %s", buf);
	} else if (type->tag == T_INT) {
		cgasm_println(ctx, "fistpl %s", buf);
	} else {
		panic("unsupported type %d", type->tag);
	}
}

struct expr_val fp_type_convert(struct cgasm_context *ctx, struct expr_val val, struct type *newtype) {
	assert(!expr_val_has_deref_flag(val)); // caller type_convert already handle deref
	struct type *oldtype = val.ctype;
	assert(is_floating_type(oldtype) || is_floating_type(newtype));

	if ((is_integer_type(oldtype) && is_floating_type(newtype)) ||
			(is_floating_type(oldtype) && is_integer_type(newtype))) {
		load_val_to_fpstk(ctx, val);
		struct expr_val temp = cgasm_alloc_temp_var(ctx, newtype);
		pop_fpstk_to_lval(ctx, temp);
		return temp;
	}

	type_dump(oldtype, 4);
	type_dump(newtype, 4);

	panic("ni");
}

// the two operands are already in fpstk.
struct expr_val cgasm_load_fp_cmp_to_temp(struct cgasm_context *ctx, int tok_tag) {
	struct expr_val temp = cgasm_alloc_temp_var(ctx, get_int_type());
	cgasm_println(ctx, "fxch %%st(1)");
	cgasm_println(ctx, "fucomip %%st(1), %%st"); // cmp st(0) with st(i)
	cgasm_println(ctx, "fstp %%st(0)"); // pop the top
	int reg = REG_EAX;
	cgasm_println(ctx, "set%s %%%s", cmp_tok_to_jmp_suffix(tok_tag, true), get_reg_str_code_size(reg, 1)); // the comparion suffix should be the unsigned version (according to the asm generated by GCC)
	cgasm_extend_reg(ctx, reg, get_char_type());
	cgasm_store_reg_to_mem(ctx, reg, temp);
	return temp;
}

struct expr_val cgasm_handle_binary_op_fp(struct cgasm_context *ctx, int tok_tag, struct expr_val lhs, struct expr_val rhs) {
	// decide the result type
	struct type *lhstype = expr_val_get_type(lhs);
	struct type *rhstype = expr_val_get_type(rhs);
	assert(is_floating_type(lhstype) || is_floating_type(rhstype));
	struct type *restype = NULL;
	if (is_floating_type(lhstype)) {
		restype = lhstype;
	}

	if (is_floating_type(rhstype) && (restype == NULL || restype->tag < rhstype->tag)) {
		restype = rhstype;
	}
	load_val_to_fpstk(ctx, lhs);
	load_val_to_fpstk(ctx, rhs);

	// compare operations for different
	if (is_cmp_tok(tok_tag)) {
		return cgasm_load_fp_cmp_to_temp(ctx, tok_tag);
	}

	struct expr_val resval = cgasm_alloc_temp_var(ctx, restype);
	(void) resval;

	switch (tok_tag) {
	case TOK_DIV:
		cgasm_println(ctx, "fdivrp"); // this is oppossite to intel manual
		break;
	case TOK_STAR:
		cgasm_println(ctx, "fmulp");
		break;
	default:
		panic("unsupported fp op %s", token_tag_str(tok_tag));
	}
	
	pop_fpstk_to_lval(ctx, resval);
	return resval;
}

// NOTE: if we are doing:
// i *= f
// we can not convert f to integer immediately. We must do the floating point
// operation (i * f) first and than store the floating point value back to i
struct expr_val cgasm_handle_assign_op_fp(struct cgasm_context *ctx, struct expr_val lhs, struct expr_val rhs, int op) {
	struct type *lhstype = expr_val_get_type(lhs);
	struct type *rhstype = expr_val_get_type(rhs);
	assert(is_floating_type(lhstype) || is_floating_type(rhstype));

	struct expr_val res = rhs;
	if (op != TOK_ASSIGN) {
		int arith_op = get_arith_from_complex_assign(op);
		res = cgasm_handle_binary_op_fp(ctx, arith_op, lhs, rhs);
	}

	// handle the simple assign: we push res to fpstk and pop the top to lhs
	load_val_to_fpstk(ctx, res);
	pop_fpstk_to_lval(ctx, lhs);
	return lhs;
}

