#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/symtab.h>
#include <inc/cbuf.h>
#include <inc/dynarr.h>

int func_alloc_space(struct cgasm_func_context *func_ctx, int size) {
	assert(size > 0);
	int nword = (size + 3) >> 2;
	func_ctx->nlocal_word += nword;
	return func_ctx->nlocal_word - 1;
}

static struct cgasm_func_context *func_context_init(const char *name) {
	struct cgasm_func_context *ctx = (struct cgasm_func_context *) mallocz(sizeof(*ctx));
	ctx->code_buf = cbuf_init();
	ctx->name = name; // no need to free when destroying, caller will do it. Normally the memory is refered by the syntree
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

	cgasm_println(ctx, ".text");
	cgasm_println_noind(ctx, ".global %s", fname); 
 	cgasm_println_noind(ctx, "%s:", fname);

	ctx->func_ctx = func_context_init(fname);
	assert(ctx->func_ctx != NULL);
	cgasm_push_symtab(ctx);
}

void cgasm_leave_function(struct cgasm_context *ctx) {
	struct cgasm_func_context *func_ctx = ctx->func_ctx;	
	int i;
	assert(func_ctx != NULL);
	ctx->func_ctx = NULL; // this avoids buffer the asm code 

	cgasm_println(ctx, "pushl %%ebp");
	cgasm_println(ctx, "movl %%esp, %%ebp");
	for (i = 0; i < ctx->nstate_reg; i++) {
		cgasm_println(ctx, "pushl %%%s", get_reg_str_code(ctx->state_reg[i]));
	}
	cgasm_println(ctx, "subl $%d, %%esp", func_ctx->nlocal_word * 4);
	cgasm_dump_buffered_code(ctx, func_ctx);

	// destroy func context
	func_context_destroy(func_ctx);

	cgasm_pop_symtab(ctx);

	// free_type_ref_in_list(ctx);
}

// XXX: does not distinguish between f() and f(void) right now
static void register_parameter(struct cgasm_context *ctx, struct parameter_declaration *decl) {
	struct type *type = parse_type_from_decl_specifiers(ctx, decl->decl_specifiers);
	char *id = NULL;
	if (decl->declarator != NULL) {
		type = parse_type_from_declarator(ctx, type, decl->declarator, &id);
	}

	if (id == NULL) {
		if (type->tag != T_VOID) {
			panic("Non void parameter without identifier");
		} else {
			type_check_ref(type);
			return;
		}
	}

	// rewrite type for parameter
	// change array to pointer
	if (type->tag == T_ARRAY) {
		struct type *newtype = get_ptr_type(type->subtype);
		type_check_ref(type);
		type = newtype;
	}

	if (type->size < 0) {
		panic("The size of symbol is undefined: %s", id);
	}
	cgasm_add_param_sym(ctx, id, type);
}

static void register_parameters(struct cgasm_context *ctx, struct dynarr *suff_list) {
	assert(dynarr_size(suff_list) == 1);
	struct direct_declarator_suffix *suff = dynarr_get(suff_list, 0);
	if (suff->param_type_list != NULL) {
		if (suff->param_type_list->has_ellipsis) {
			red("ellipsis is just ignored right now");
		}

		DYNARR_FOREACH_BEGIN(suff->param_type_list->param_decl_list, parameter_declaration, each);
			register_parameter(ctx, each);
		DYNARR_FOREACH_END();
	}
}

void cgasm_function_definition(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers, struct declarator *func_def_declarator, struct compound_statement *compound_stmt) {	
	char *fname = NULL;
	struct type *func_type = parse_type_from_decl_specifiers(ctx, decl_specifiers);
	func_type = parse_type_from_declarator(ctx, func_type, func_def_declarator, &fname);

	if (fname == NULL) {
		panic("function definition without name");
	}
	if (func_type->tag != T_FUNC) {
		panic("requires function type");
	}
	// register function declaration
	cgasm_add_decl_sym(ctx, fname, func_type);


	cgasm_enter_function(ctx, fname);

	// handle parameters
	register_parameters(ctx, func_def_declarator->direct_declarator->suff_list);

	// rewrite the syntax tree if we need add return statement
	add_return_cond(compound_stmt);	

	cgasm_compound_statement(ctx, compound_stmt);
	cgasm_leave_function(ctx);
}
