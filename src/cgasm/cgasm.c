#include <stdarg.h>

#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/syntree.h>
#include <inc/syntree-node.h>
#include <inc/symtab.h>

static void cgasm_translation_unit(struct cgasm_context *ctx, struct translation_unit *trans_unit);

static struct cgasm_context *cgasm_context_init(FILE *fp) {
	struct cgasm_context *ctx = mallocz(sizeof(*ctx));
	ctx->fp = fp;
	ctx->top_stab = symtab_init(NULL);
	ctx->str_literals = dynarr_init();

	ctx->state_reg[0] = REG_ESI;
	ctx->state_reg[1] = REG_EDI;
	ctx->state_reg[2] = REG_EBX;
	ctx->nstate_reg = 3;

	return ctx;
}

static void cgasm_context_destroy(struct cgasm_context *ctx) {
	cgasm_destroy_str_literals(ctx);
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

void cgasm_declaration(struct cgasm_context *ctx, struct declaration_specifiers *decl_specifiers, struct init_declarator_list *init_declarator_list) {
	// function declaration
	if (is_func_decl_init_declarator_list(init_declarator_list)) {
		return; // XXX ignore function declaration right now
	}

	struct dynarr *idlist = extract_id_list_from_init_declarator_list(init_declarator_list);
	DYNARR_FOREACH_PLAIN_BEGIN(idlist, char *, each);
		cgasm_add_decl_sym(ctx, each);
	DYNARR_FOREACH_END();
}

static void cgasm_external_declaration(struct cgasm_context *ctx, struct external_declaration *external_decl) {
	if (external_decl->func_def_declarator != NULL) {
		cgasm_function_definition(ctx, external_decl->decl_specifiers, external_decl->func_def_declarator, external_decl->compound_stmt);
	} else {
		cgasm_declaration(ctx, external_decl->decl_specifiers, external_decl->init_declarator_list);		
	}
}

static void cgasm_leave_translation_unit(struct cgasm_context *ctx) {
	cgasm_dump_string_literals(ctx); 
}

static void cgasm_translation_unit(struct cgasm_context *ctx, struct translation_unit *trans_unit) {
	DYNARR_FOREACH_BEGIN(trans_unit->external_decl_list, external_declaration, each);
		cgasm_external_declaration(ctx, each);
	DYNARR_FOREACH_END();
	cgasm_leave_translation_unit(ctx);
}
