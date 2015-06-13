#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <inc/cgc.h>
#include <inc/syntree.h>
#include <inc/util.h>

struct cgc_context {
	FILE *fp;
	int indent;
	int step; // the step for indent
};

static void cgc_translation_unit(struct cgc_context *ctx, struct translation_unit *trans_unit);
static void cgc_external_declaration(struct cgc_context *ctx, struct external_declaration *external_decl);
static void cgc_function_definition(struct cgc_context *ctx, struct declaration_specifiers *decl_specifiers, struct declarator *func_def_declarator, struct compound_statement *compound_stmt);
static void cgc_declaration(struct cgc_context *ctx, struct declaration_specifiers *decl_specifiers, struct init_declarator_list *init_declarator_list);
static void cgc_declaration_specifiers(struct cgc_context *ctx, struct declaration_specifiers *decl_specifiers);
static void cgc_type_specifier(struct cgc_context *ctx, struct type_specifier *type_specifier);
static void cgc_declarator(struct cgc_context *ctx, struct declarator *declarator);
static void cgc_direct_declarator(struct cgc_context *ctx, struct direct_declarator *direct_declarator);
static void cgc_compound_statement(struct cgc_context *ctx, struct compound_statement *compound_stmt);
static void cgc_statement(struct cgc_context *ctx, struct syntreebasenode *stmt);
static void cgc_init_declarator_list(struct cgc_context *ctx, struct init_declarator_list *init_declarator_list);
static void cgc_init_declarator(struct cgc_context *ctx, struct init_declarator *init_declarator);

// this method will take care of the indent
static void cgc_printi(struct cgc_context *ctx, const char *fmt, ...) {
	va_list va;
	fprintf(ctx->fp, "%*s", ctx->indent, "");
	va_start(va, fmt);
	vfprintf(ctx->fp, fmt, va);
	va_end(va);
}

// this method does not add indent
static void cgc_print(struct cgc_context *ctx, const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	vfprintf(ctx->fp, fmt, va);
	va_end(va);
}

static void cgc_indent(struct cgc_context *ctx) {
	fprintf(ctx->fp, "%*s", ctx->indent, "");
}

struct cgc_context *cgc_context_init(FILE *fp, int indent) {
	struct cgc_context *ctx = mallocz(sizeof(*ctx));
	ctx->fp = fp;
	ctx->indent = indent;
	ctx->step = 2;
	return ctx;
}

void cgc_context_destroy(struct cgc_context *ctx) {
	free(ctx);
}

void cgc_tree(struct cgc_context *ctx, struct syntree *tree) {
	cgc_translation_unit(ctx, tree->trans_unit);
}

static void cgc_translation_unit(struct cgc_context *ctx, struct translation_unit *trans_unit) {
	DYNARR_FOREACH_BEGIN(trans_unit->external_decl_list, external_declaration, each);
		cgc_external_declaration(ctx, each);
	DYNARR_FOREACH_END();
}

static void cgc_external_declaration(struct cgc_context *ctx, struct external_declaration *external_decl) {
	if (external_decl->func_def_declarator != NULL) {
		cgc_function_definition(ctx, external_decl->decl_specifiers, external_decl->func_def_declarator, external_decl->compound_stmt);
	} else {
		cgc_declaration(ctx, external_decl->decl_specifiers, external_decl->init_declarator_list);
	}
}

static void cgc_function_definition(struct cgc_context *ctx, struct declaration_specifiers* decl_specifiers, struct declarator *func_def_declarator, struct compound_statement *compound_stmt) {
	cgc_declaration_specifiers(ctx, decl_specifiers);
	cgc_print(ctx, " ");
	cgc_declarator(ctx, func_def_declarator);
	cgc_print(ctx, "\n");
	cgc_compound_statement(ctx, compound_stmt);
}

static void cgc_compound_statement(struct cgc_context *ctx, struct compound_statement *compound_stmt) {
	cgc_indent(ctx); cgc_print(ctx, "{\n");

	ctx->indent += ctx->step;
	DYNARR_FOREACH_BEGIN(compound_stmt->declList, declaration, each);
		cgc_declaration(ctx, each->decl_specifiers, each->init_declarator_list);
	DYNARR_FOREACH_END();

	DYNARR_FOREACH_BEGIN(compound_stmt->stmtList, syntreebasenode, each);
		cgc_statement(ctx, each);
	DYNARR_FOREACH_END();

	ctx->indent -= ctx->step;
	cgc_indent(ctx); cgc_print(ctx, "}\n");
}

static void cgc_statement(struct cgc_context *ctx, struct syntreebasenode *stmt) {
	panic("ni");
}

static void cgc_declarator(struct cgc_context *ctx, struct declarator *declarator) {
	cgc_direct_declarator(ctx, declarator->direct_declarator);
}

static void cgc_direct_declarator(struct cgc_context *ctx, struct direct_declarator *direct_declarator) {
	assert(direct_declarator->id != NULL);
	cgc_print(ctx, "%s", direct_declarator->id);
	// TODO
}

static void cgc_declaration(struct cgc_context *ctx, struct declaration_specifiers *decl_specifiers, struct init_declarator_list *init_declarator_list) {
	cgc_indent(ctx);
	cgc_declaration_specifiers(ctx, decl_specifiers);
	cgc_print(ctx, " ");
	cgc_init_declarator_list(ctx, init_declarator_list);
	cgc_print(ctx, ";\n");
}

static void cgc_init_declarator_list(struct cgc_context *ctx, struct init_declarator_list *init_declarator_list) {
	int is_first = 1;
	DYNARR_FOREACH_BEGIN(init_declarator_list->darr, init_declarator, each);
		if (!is_first) {
			cgc_print(ctx, ", ");
		}
		is_first = 0;
		cgc_init_declarator(ctx, each);
	DYNARR_FOREACH_END();
}

static void cgc_init_declarator(struct cgc_context *ctx, struct init_declarator *init_declarator) {
	cgc_declarator(ctx, init_declarator->declarator);
	assert(init_declarator->initializer == NULL); // not supported yet
}

static void cgc_declaration_specifiers(struct cgc_context *ctx, struct declaration_specifiers *decl_specifiers) {
	cgc_indent(ctx);
	DYNARR_FOREACH_BEGIN(decl_specifiers->darr, syntreebasenode, each);
		switch (each->nodeType) {
		case TYPE_SPECIFIER:
			cgc_type_specifier(ctx, (struct type_specifier *) each);
			break;
		default:
			panic("ni");
		}
	DYNARR_FOREACH_END();
}

static void cgc_type_specifier(struct cgc_context *ctx, struct type_specifier *type_specifier) {
	switch (type_specifier->tok_tag) {
	case TOK_INT:
		cgc_print(ctx, "int");
		break;
	default:
		panic("ni");
	}
}

