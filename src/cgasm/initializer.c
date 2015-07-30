#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/syntree.h>
#include <inc/symtab.h>

static void cgasm_initialize_global_vint(struct cgasm_context *ctx, struct global_var_symbol *sym, struct initializer *initializer, const char *directive) {
	struct assignment_expression *expr = initializer->expr;
	if (expr == NULL) {
		panic("inavlid initializer");
	}
	struct expr_val val = cgasm_assignment_expression(ctx, expr);
	int const_val = cgasm_get_int_const_from_expr(ctx, val);
	cgasm_println_noind(ctx, "%s:", sym->name);
	cgasm_println(ctx, "%s %d", directive, const_val);
}

static void cgasm_initialize_global_var(struct cgasm_context *ctx, struct global_var_symbol *sym, struct initializer *initializer) {
	struct type *type = sym->ctype;

	switch (type->tag) {
	case T_INT:
		cgasm_initialize_global_vint(ctx, sym, initializer, ".long");
		break;
	case T_SHORT:
		cgasm_initialize_global_vint(ctx, sym, initializer, ".word");
		break;
	case T_CHAR:
		cgasm_initialize_global_vint(ctx, sym, initializer, ".byte");
		break;
	default:
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
	cgasm_initialize_global_var(ctx, sym, initializer);
}
