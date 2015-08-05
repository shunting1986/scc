#include <inc/util.h>
#include <inc/fp.h>
#include <inc/type.h>
#include <inc/cgasm.h>

static void load_int_to_fpstk(struct cgasm_context *ctx, struct expr_val val) {
	int reg = REG_EAX;
	cgasm_load_val_to_reg(ctx, val, reg);
	cgasm_println(ctx, "fildl %%%s", get_reg_str_code(reg));
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
	panic("ni");
}


