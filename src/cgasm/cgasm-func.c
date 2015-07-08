#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/symtab.h>
#include <inc/cbuf.h>

static struct cgasm_func_context *func_context_init() {
	struct cgasm_func_context *ctx = (struct cgasm_func_context *) mallocz(sizeof(*ctx));
	ctx->code_buf = cbuf_init();
	return ctx;
}

static void func_context_destroy(struct cgasm_func_context *ctx) {
	cbuf_destroy(ctx->code_buf);	
	free(ctx);
}

static void cgasm_dump_buffered_code(struct cgasm_context *ctx, struct cgasm_func_context *func_ctx) {
	struct cbuf *cbuf = func_ctx->code_buf;
	char *s;
	s = cbuf_transfer(cbuf);
	cgasm_println_noind(ctx, "%s", s);
	free(s);
}

/*
 * create function context
 */
void cgasm_enter_function(struct cgasm_context *ctx, char *fname) {
	if (ctx->func_ctx != NULL) {
		panic("nested function definition not allowed");
	}

	cgasm_println_noind(ctx, ".global %s", fname); 
 	cgasm_println_noind(ctx, "%s:", fname);

	ctx->func_ctx = func_context_init();
	ctx->top_stab = symtab_init(ctx->top_stab);
}

void cgasm_leave_function(struct cgasm_context *ctx) {
	struct cgasm_func_context *func_ctx = ctx->func_ctx;	
	int i;
	assert(func_ctx != NULL);
	ctx->func_ctx = NULL; // this avoids buffer the asm code 

	cgasm_println(ctx, "pushl %%ebp");
	cgasm_println(ctx, "movl %%ebp, %%esp");
	for (i = 0; i < ctx->nstate_reg; i++) {
		cgasm_println(ctx, "pushl %%%s", get_reg_str_code(ctx->state_reg[i]));
	}
	cgasm_println(ctx, "subl %%esp, $%d", func_ctx->nlocal_word * 4);
	cgasm_dump_buffered_code(ctx, func_ctx);

	panic("ni"); // TODO
	 
	// destroy func context
	func_context_destroy(func_ctx);

	{
		// pop the top symtab
		struct symtab *old_stab = ctx->top_stab;
		ctx->top_stab = old_stab->enclosing;
		symtab_destroy(old_stab);
	}
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
	cgasm_enter_function(ctx, fname);

	// handle parameters
	register_parameters(ctx, func_def_declarator->direct_declarator->suff_list);
	cgasm_compound_statement(ctx, compound_stmt);
	cgasm_leave_function(ctx);
}
