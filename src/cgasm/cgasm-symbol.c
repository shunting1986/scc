#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/symtab.h>

void cgasm_push_symtab(struct cgasm_context *ctx) {
	ctx->top_stab = symtab_init(ctx->top_stab);
}

void cgasm_pop_symtab(struct cgasm_context *ctx) {
	// pop the top symtab
	struct symtab *old_stab = ctx->top_stab;
	assert(old_stab != NULL);
	ctx->top_stab = old_stab->enclosing;
	symtab_destroy(old_stab);
}

static void cgasm_check_sym_redef(struct cgasm_context *ctx, char *id) {
	if (symtab_lookup_norec(ctx->top_stab, id) != NULL) {
		panic("symbol redefinition: %s", id);
	}
}

struct symbol *cgasm_lookup_sym(struct cgasm_context *ctx, char *id) {
	struct symbol *sym = symtab_lookup(ctx->top_stab, id);
	if (sym == NULL) {
		panic("symbol undefined: %s", id);
	}
	return sym;
}

struct type *cgasm_get_type_from_type_name(struct cgasm_context *ctx, char *id) {
	struct symbol *sym = cgasm_lookup_sym(ctx, id);
	return sym->ctype;
}

struct symbol *cgasm_add_decl_sym(struct cgasm_context *ctx, char *id, struct type *type) {
	struct cgasm_func_context *func_ctx =  ctx->func_ctx;	
	struct symbol *ret;
	cgasm_check_sym_redef(ctx, id);
	if (func_ctx == NULL) {
		assert(ctx->top_stab->enclosing == NULL);
		symtab_add(ctx->top_stab, ret = symtab_new_global_var(id, type));
	} else {
		assert(ctx->top_stab->enclosing != NULL);	
		symtab_add(ctx->top_stab, ret = symtab_new_local_var(id, func_alloc_space(func_ctx, type->size), type)); // idx start from 0
	}
	return ret;
}

// ind start from 0
void cgasm_add_param_sym(struct cgasm_context *ctx, char *id, struct type *type) {
	struct cgasm_func_context *func_ctx =  ctx->func_ctx;	
	int ind;
	assert(func_ctx != NULL);
	// assert(ind < func_ctx->nparam_word);
	assert(ctx->top_stab->enclosing != NULL);
	cgasm_check_sym_redef(ctx, id);
	ind = func_ctx->nparam_word++;
	// XXX assume the size of parameter is 4 bytes right now
	symtab_add(ctx->top_stab, symtab_new_param(id, ind, type));
}

static void cgasm_dump_one_global_var(void *_ctx, const char *key, void *_val) {
	struct cgasm_context *ctx = _ctx;
	struct global_var_symbol *sym = _val;
	assert(sym->type == SYMBOL_GLOBAL_VAR);
	(void) ctx;

	// TODO does not consider initializer yet
	assert(sym->ctype != NULL);
	cgasm_println(ctx, ".comm %s, %d, %d", sym->name, sym->ctype->size, 4); // XXX hardcode to 4 byte alignment right now
}

/*
 * dump the definition of global variables
 */
void cgasm_dump_global_vars(struct cgasm_context *ctx) {
	struct symtab *stab = ctx->top_stab;
	assert(stab != NULL);
	assert(stab->enclosing == NULL);

	/* 
	 * This is very implement. Missing this will still generate compilable code,
	 * but since the data is actually reside in text segment, whenever the code
	 * attemp to write the location, the prog will trigger segmentation fault.
	 */
	cgasm_println(ctx, ".data"); 

	symtab_iter(stab, ctx, cgasm_dump_one_global_var);
}


