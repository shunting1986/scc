#include <inc/type.h>
#include <inc/util.h>
#include <inc/cgasm.h>

static struct expr_val type_convert_int_to_ll(struct cgasm_context *ctx, struct expr_val val) {
	assert(val.ctype->tag == T_INT);
	if (val.type == EXPR_VAL_CONST_VAL) {
		val.ctype = get_long_long_type();
		val.const_val = wrap_ll_const_to_token(val.const_val.const_val.ival);
		return val;
	}

	// panic("type_conv");

	// allocate an temp var
	struct expr_val temp = cgasm_alloc_temp_var(ctx, get_long_long_type());
	int offset = cgasm_get_temp_var_offset(ctx, temp.temp_var);
	int reg = REG_EAX;
	cgasm_load_val_to_reg(ctx, val, reg);
	cgasm_println(ctx, "movl %%%s, %d(%%ebp)", get_reg_str_code(reg), offset);
	cgasm_println(ctx, "movl $0, %d(%%ebp)", offset + 4);
	return temp;
}

// NOTE: caller will handle the type ref for newtype
struct expr_val type_convert(struct cgasm_context *ctx, struct expr_val val, struct type *newtype) {
	assert(val.ctype != NULL);
	val = cgasm_handle_deref_flag(ctx, val); // the DEREF flag is already handled here

	struct type *oldtype = val.ctype;

	if (oldtype->tag >= T_CHAR && oldtype->tag <= T_LONG_LONG && newtype->tag >= T_CHAR && newtype->tag <= T_LONG_LONG) { // both are integer type
		if (oldtype->tag >= newtype->tag) { // change to smaller type
			// nothing need to do
		} else if (oldtype->tag == T_INT && newtype->tag == T_LONG_LONG) { // TODO int to long long is not supported yet
			return type_convert_int_to_ll(ctx, val);
		} else {
			// XXX need do zero or sign extension
			panic("ni");
		}
	} else if (oldtype->tag == T_INT && is_void_ptr(newtype)) { // int to void *
		// nothing need to do
	} else if (newtype->tag == T_VOID) { // (void), discard value
	} else if (oldtype->tag == T_PTR && newtype->tag == T_PTR) { // change from one pointer type to another (can cover func ptr case)
	} else if (oldtype->tag == T_INT && newtype->tag == T_PTR) { // convert from int to ptr
	} else {
		type_dump(oldtype, 4);
		type_dump(newtype, 4);
		panic("ni");
	}

	val.ctype = newtype;
	return val;
}
