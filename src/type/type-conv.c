#include <inc/type.h>
#include <inc/util.h>
#include <inc/cgasm.h>
#include <inc/fp.h>

static struct expr_val type_convert_integer_to_ll(struct cgasm_context *ctx, struct expr_val val) {
	struct type *oldtype = expr_val_get_type(val);
	assert(is_integer_type(oldtype));
	if (oldtype->tag == T_LONG_LONG) {
		return val;
	}

	// old type is char, short, int
	if (val.type == EXPR_VAL_CONST_VAL) {
		val.ctype = get_long_long_type();
		val.const_val = wrap_ll_const_to_token(val.const_val.const_val.ival);
		return val;
	}

	// allocate an temp var
	struct expr_val temp = cgasm_alloc_temp_var(ctx, get_long_long_type());
	int offset = cgasm_get_temp_var_offset(ctx, temp.temp_var);
	int reg = REG_EAX;
	cgasm_load_val_to_reg(ctx, val, reg);

	if (oldtype->size != 4) {
		cgasm_println(ctx, "movl $0, %d(%%ebp)", offset);
	}
	cgasm_println(ctx, "mov%s %%%s, %d(%%ebp)", size_to_suffix(oldtype->size), get_reg_str_code_size(reg, oldtype->size), offset);
	cgasm_println(ctx, "movl $0, %d(%%ebp)", offset + 4);
	return temp;
}

// the new type should not be int64 since that has already been handled
struct expr_val extend_int_type(struct cgasm_context *ctx, struct expr_val val, struct type *newtype) {
	assert(is_integer_type(newtype));
	assert(newtype->tag != T_LONG_LONG);

	char buf[256];

	// TODO assume zero extension right now..
	// TODO if val is already a temp var, we do not need to allocate another one
	struct expr_val ret = cgasm_alloc_temp_var(ctx, newtype);
	cgasm_println(ctx, "movl $0, %s", cgasm_get_lval_asm_code(ctx, ret, buf, NULL));
	int reg = REG_EAX;
	cgasm_load_val_to_reg(ctx, val, reg);

	ret.ctype = val.ctype; // make sure we only copy the part we want
	cgasm_store_reg_to_mem(ctx, reg, ret); 
	ret.ctype = newtype;
	return ret;
}

// NOTE: caller will handle the type ref for newtype
struct expr_val type_convert(struct cgasm_context *ctx, struct expr_val val, struct type *newtype) {
	assert(val.ctype != NULL);
	val = cgasm_handle_deref_flag(ctx, val); // the DEREF flag is already handled here

	// for array, the deref flag is not removed
	struct type *oldtype = expr_val_get_type(val);

	if (is_integer_type(oldtype) && is_integer_type(newtype)) { // both are integer type
		if (oldtype->tag >= newtype->tag) { // change to smaller type
			// nothing need to do
		} else if (newtype->tag == T_LONG_LONG) { 
			return type_convert_integer_to_ll(ctx, val);
		} else {
			return extend_int_type(ctx, val, newtype);
		}
	} else if (oldtype->tag == T_INT && is_void_ptr(newtype)) { // int to void *
		// nothing need to do
	} else if (newtype->tag == T_VOID) { // (void), discard value
	} else if (oldtype->tag == T_PTR && newtype->tag == T_PTR) { // change from one pointer type to another (can cover func ptr case)
	} else if (oldtype->tag == T_INT && newtype->tag == T_PTR) { // convert from int to ptr
	} else if (oldtype->tag == T_PTR && newtype->tag == T_INT) { // convert from ptr to int
	} else if (oldtype->tag == T_ARRAY && newtype->tag == T_PTR) {
		// XXX we'd better have an extra parameter to determine if this is implicit or 
		// explicit conversion
	#if 0
		if (!type_eq(oldtype->subtype, newtype->subtype)) {
			type_dump(oldtype->subtype, 4);
			type_dump(newtype->subtype, 4);
		}
	#endif
		cgasm_change_array_func_to_ptr(ctx, &val);
		return val;
	} else if (is_floating_type(oldtype) || is_floating_type(newtype)) {
		return fp_type_convert(ctx, val, newtype);
	} else if (oldtype->tag == T_FUNC && is_func_ptr(newtype)) { // func to func ptr
		cgasm_change_array_func_to_ptr(ctx, &val);	
		// the following code will set the func ptr type
	} else {
		type_dump(oldtype, 4);
		type_dump(newtype, 4);
		assert(0);
		panic("ni");
	}

	val.ctype = newtype;
	return val;
}
