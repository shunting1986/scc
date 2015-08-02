#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/syntree.h>
#include <inc/symtab.h>
#include <inc/cgc.h>

static void cgasm_initialize_global_var(struct cgasm_context *ctx, struct type *type, struct initializer *initializer);

static void cgasm_initialize_global_vint(struct cgasm_context *ctx, struct initializer *initializer, const char *directive) {
	struct assignment_expression *expr = initializer->expr;
	if (expr == NULL) {
		panic("inavlid initializer");
	}
	struct expr_val val = cgasm_assignment_expression(ctx, expr);
	int const_val = cgasm_get_int_const_from_expr(ctx, val);
	cgasm_println(ctx, "%s %d", directive, const_val);
}

static void cgasm_initialize_global_ptr(struct cgasm_context *ctx, struct type *type, struct initializer *initializer) {
	char buf[256];
	assert(type->tag == T_PTR);

	// initialize char ptr
	if (type->subtype->tag == T_CHAR) { 
		if (initializer->expr != NULL) {
			struct expr_val val = cgasm_assignment_expression(ctx, initializer->expr);
			if (val.type == EXPR_VAL_STR_LITERAL) { // use string literal to initialize char *
				cgasm_println(ctx, ".long %s", get_str_literal_label(val.ind, buf));
				return;
			} else {
				panic("ni");
			}
		} else {
			panic("initializer list case ");
		}
	}
	panic("not initializing char *");
}

// TODO this function should make sure type's size is > 0 before it returns
static void cgasm_initialize_global_array(struct cgasm_context *ctx, struct type *type, struct initializer *initializer) {
	if (type->dim < 0 && initializer->initz_list != NULL) {
		complete_array_dim(type, dynarr_size(initializer->initz_list->list));
	}

	if (type->size < 0) {
		panic("The size of array is undefined");
	}

	int i;
	struct dynarr *list = initializer->initz_list->list;
	int size = dynarr_size(list);
	if (type->dim < size) {
		panic("The array dimension is too small");
	}

	for (i = 0; i < size; i++) {
		cgasm_initialize_global_var(ctx, type->subtype, dynarr_get(list, i));
	}

	if (size < type->dim) {
		cgasm_println(ctx, ".zero %d", type->subtype->size * (type->dim - size));
	}
}

static void cgasm_initialize_global_var(struct cgasm_context *ctx, struct type *type, struct initializer *initializer) {
	if (initializer == NULL) {
		if (type->size < 0) {
			panic("type not complete");
		}
		cgasm_println(ctx, ".zero %d", type->size);
		return;
	}

	switch (type->tag) {
	case T_INT:
		cgasm_initialize_global_vint(ctx, initializer, ".long");
		break;
	case T_SHORT:
		cgasm_initialize_global_vint(ctx, initializer, ".word");
		break;
	case T_CHAR:
		cgasm_initialize_global_vint(ctx, initializer, ".byte");
		break;
	case T_PTR:
		cgasm_initialize_global_ptr(ctx, type, initializer);
		break;
	case T_ARRAY:
		cgasm_initialize_global_array(ctx, type, initializer);
		break;
	default:
		cgc_dump(initializer, initializer);
		panic("ni %d", type->tag);
	}
}

void cgasm_allocate_global_var(struct cgasm_context *ctx, struct global_var_symbol *sym, struct initializer *initializer) {
	// no need to allocate space for typedef/extern symbol
	if (sym->flags & (SYMBOL_FLAG_TYPEDEF | SYMBOL_FLAG_EXTERN)) {
		return;
	}

	// no need to allocate space for func declaration
	if (sym->ctype->tag == T_FUNC) {
		return; 
	}

	cgasm_println(ctx, ".data");
	if (initializer == NULL) {
		cgasm_println(ctx, ".comm %s, %d, %d", sym->name, sym->ctype->size, 4); // XXX hardcode to 4 byte alignment right now
		return;
	}

	// global variable with initializer
	cgasm_println_noind(ctx, "%s:", sym->name);
	cgasm_initialize_global_var(ctx, sym->ctype, initializer);
}
