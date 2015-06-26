#include <stdarg.h>

#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/syntree.h>
#include <inc/syntree-node.h>

static void cgasm_translation_unit(struct cgasm_context *ctx, struct translation_unit *trans_unit);

static struct cgasm_context *cgasm_context_init(FILE *fp) {
	struct cgasm_context *ctx = mallocz(sizeof(*ctx));
	ctx->fp = fp;
	return ctx;
}

static void cgasm_context_destroy(struct cgasm_context *ctx) {
	free(ctx);
}

/**
 * create context in this method
 */
void cgasm_tree(struct syntree *tree) {
	struct cgasm_context *ctx = cgasm_context_init(stdout);	
	cgasm_translation_unit(ctx, tree->trans_unit);
	cgasm_context_destroy(ctx);
}

static int is_func_decl_direct_declarator_suffix(struct direct_declarator_suffix *suff) {
	return suff->empty_paren || suff->param_type_list != NULL;
}

static int is_func_decl_declarator(struct declarator *declarator) {
	struct direct_declarator *direct_declarator = declarator->direct_declarator;
	if (direct_declarator->id == NULL) {
		return 0;
	}
	struct dynarr *darr = direct_declarator->suff_list;
	return dynarr_size(darr) == 1 && is_func_decl_direct_declarator_suffix(dynarr_get(darr, 0));
}

static int is_func_decl_init_declarator_list(struct init_declarator_list *init_declarator_list) {
	struct dynarr *darr = init_declarator_list->darr;
	if (dynarr_size(darr) != 1) {
		return 0;
	}
	struct init_declarator *init_declarator = dynarr_get(darr, 0);
	return init_declarator->initializer == NULL && is_func_decl_declarator(init_declarator->declarator);
}

static void cgasm_declaration(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers, struct init_declarator_list *init_declarator_list) {
	// function declaration
	if (is_func_decl_init_declarator_list(init_declarator_list)) {
		return; // XXX ignore function declaration right now
	}

	panic("ni");
}

static void cgasm_external_declaration(struct cgasm_context *ctx, struct external_declaration *external_decl) {
	if (external_decl->func_def_declarator != NULL) {
		cgasm_function_definition(ctx, external_decl->decl_specifiers, external_decl->func_def_declarator, external_decl->compound_stmt);
	} else {
		cgasm_declaration(ctx, external_decl->decl_specifiers, external_decl->init_declarator_list);		
	}
}

static void cgasm_translation_unit(struct cgasm_context *ctx, struct translation_unit *trans_unit) {
	DYNARR_FOREACH_BEGIN(trans_unit->external_decl_list, external_declaration, each);
		cgasm_external_declaration(ctx, each);
	DYNARR_FOREACH_END();
}
