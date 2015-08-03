#include <inc/type.h>
#include <inc/util.h>

// NOTE: caller will handle the type ref for newtype
struct expr_val type_convert(struct cgasm_context *ctx, struct expr_val val, struct type *newtype) {
	assert(val.ctype != NULL);
	val = cgasm_handle_deref_flag(ctx, val);

	struct type *oldtype = val.ctype;

	if (oldtype->tag >= T_CHAR && oldtype->tag <= T_LONG_LONG && newtype->tag >= T_CHAR && newtype->tag <= T_LONG_LONG) { // both are integer type
		if (oldtype->tag >= newtype->tag) { // change to smaller type
			// nothing need to do
		} else {
			// XXX need do zero or sign extension
			panic("ni");
		}
	} else if (oldtype->tag == T_INT && is_void_ptr(newtype)) { // int to void *
		// nothing need to do
	} else if (newtype->tag == T_VOID) { // (void), discard value
	} else if (oldtype->tag == T_PTR && newtype->tag == T_PTR) { // change from one pointer type to another (can cover func ptr case)
	} else {
		type_dump(oldtype, 4);
		type_dump(newtype, 4);
		panic("ni");
	}

	val.ctype = newtype;
	return val;
}
