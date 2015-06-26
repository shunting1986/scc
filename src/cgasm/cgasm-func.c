#include <inc/cgasm.h>
#include <inc/util.h>

struct cgasm_func_context {
	int nparam_word; // number of 32bit words for parameter
	int nlocal_word; // number of 32bit words for local variables
};

static struct cgasm_func_context *func_context_init() {
	struct cgasm_func_context *ctx = (struct cgasm_func_context *) mallocz(sizeof(*ctx));
	return ctx;
}

static void func_context_destroy(struct cgasm_func_context *ctx) {
	free(ctx);
}

/*
 * create function context
 */
void cgasm_enter_function(struct cgasm_context *ctx) {
	if (ctx->func_ctx != NULL) {
		panic("nested function definition not allowed");
	}
	ctx->func_ctx = func_context_init();
}

void cgasm_leave_function(struct cgasm_context *ctx) {
	assert(ctx->func_ctx != NULL);
	free(ctx->func_ctx);
	ctx->func_ctx = NULL;
}

void cgasm_function_definition(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers, struct declarator *func_def_declarator, struct compound_statement *compound_stmt) {	
	char *fname = func_def_declarator->direct_declarator->id;
	cgasm_println(ctx, ".global %s", fname); // XXX can do this when leaving the function
	cgasm_println(ctx, "%s:", fname);
	cgasm_enter_function(ctx);

	// middle
	panic("ni");

	cgasm_leave_function(ctx);
}
