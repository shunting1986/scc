#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/symtab.h>

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
	ctx->top_stab = symtab_init(ctx->top_stab);
}

void cgasm_leave_function(struct cgasm_context *ctx) {
	panic("ni");
	assert(ctx->func_ctx != NULL);
	free(ctx->func_ctx);
	ctx->func_ctx = NULL;

	// pop the top symtab
	struct symtab *old_stab = ctx->top_stab;
	ctx->top_stab = old_stab->enclosing;
	symtab_destroy(old_stab);
}

static void register_parameters(struct cgasm_context *ctx, struct dynarr *suff_list) {
	assert(dynarr_size(suff_list) == 1);
	struct direct_declarator_suffix *suff = dynarr_get(suff_list, 0);
	if (suff->param_type_list != NULL) {
		panic("ni");
	}
}

void cgasm_function_definition(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers, struct declarator *func_def_declarator, struct compound_statement *compound_stmt) {	
	char *fname = func_def_declarator->direct_declarator->id;
	cgasm_println(ctx, ".global %s", fname); // XXX can do this when leaving the function
	cgasm_println(ctx, "%s:", fname);
	cgasm_enter_function(ctx);

	// handle parameters
	register_parameters(ctx, func_def_declarator->direct_declarator->suff_list);
	cgasm_compound_statement(ctx, compound_stmt);
	cgasm_leave_function(ctx);
}
