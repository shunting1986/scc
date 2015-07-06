#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/symtab.h>

static void cgasm_check_sym_redef(struct cgasm_context *ctx, char *id) {
	if (symtab_lookup_norec(ctx->top_stab, id) != NULL) {
		panic("symbol redefinition: %s", id);
	}
}

void cgasm_add_decl_sym(struct cgasm_context *ctx, char *id) {
	struct cgasm_func_context *func_ctx =  ctx->func_ctx;	
	cgasm_check_sym_redef(ctx, id);
	if (func_ctx == NULL) {
		assert(ctx->top_stab->enclosing == NULL);
		symtab_add(ctx->top_stab, symtab_new_global_var(id));
	} else {
		assert(ctx->top_stab->enclosing != NULL);	
		symtab_add(ctx->top_stab, symtab_new_local_var(id, func_ctx->nlocal_word++)); // idx start from 0
	}
}

// ind start from 0
void cgasm_add_param_sym(struct cgasm_context *ctx, char *id, int ind) {
	struct cgasm_func_context *func_ctx =  ctx->func_ctx;	
	assert(func_ctx != NULL);
	assert(ind < func_ctx->nparam_word);
	assert(ctx->top_stab->enclosing != NULL);
	symtab_add(ctx->top_stab, symtab_new_param(id, ind));
}

