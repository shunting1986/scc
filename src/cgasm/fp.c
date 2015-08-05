#include <inc/util.h>
#include <inc/fp.h>
#include <inc/type.h>
#include <inc/cgasm.h>

static void load_int_to_fpstk(struct cgasm_context *ctx, struct expr_val val);
static void load_ll_to_fpstk(struct cgasm_context *ctx, struct expr_val val);

void cgasm_push_fp_val_to_gstk(struct cgasm_context *ctx, struct expr_val val) {
	struct type *type = expr_val_get_type(val);
	int addr_reg = REG_EAX;
	assert(is_floating_type(type));
	cgasm_load_addr_to_reg(ctx, val, addr_reg);
	cgasm_push_bytes(ctx, addr_reg, 0, type_get_size(type));
}

static void load_val_to_fpstk(struct cgasm_context *ctx, struct expr_val val) {
	struct type *type = expr_val_get_type(val);
	int addr_reg = REG_EAX;
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

static void pop_fpstk_to_lval(struct cgasm_context *ctx, struct expr_val dst) {
	struct type *type = dst.ctype;
	assert(is_floating_type(type));
	char buf[256];
	cgasm_get_lval_asm_code(ctx, dst, buf);
	if (type->tag == T_FLOAT) {
		cgasm_println(ctx, "fstps %s", buf);
	} else {
		cgasm_println(ctx, "fstpl %s", buf);
	}
}

struct expr_val fp_type_convert(struct cgasm_context *ctx, struct expr_val val, struct type *newtype) {
	assert(!expr_val_has_deref_flag(val)); // caller type_convert already handle deref
	struct type *oldtype = val.ctype;
	assert(is_floating_type(oldtype) || is_floating_type(newtype));

	if (oldtype->tag == T_INT && newtype->tag == T_DOUBLE) {
		load_int_to_fpstk(ctx, val);
		struct expr_val temp = cgasm_alloc_temp_var(ctx, get_double_type());
		pop_fpstk_to_lval(ctx, temp);
		return temp;
	}

	type_dump(oldtype, 4);
	type_dump(newtype, 4);

	panic("ni");
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
	struct expr_val resval = cgasm_alloc_temp_var(ctx, restype);
	(void) resval;
	load_val_to_fpstk(ctx, lhs);
	load_val_to_fpstk(ctx, rhs);

	switch (tok_tag) {
	case TOK_DIV:
		cgasm_println(ctx, "fdivrp"); // this is oppossite to intel manual
		break;
	default:
		panic("unsupported fp op %s", token_tag_str(tok_tag));
	}
	
	pop_fpstk_to_lval(ctx, resval);
	return resval;
}


