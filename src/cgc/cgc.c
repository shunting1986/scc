#include <stdio.h>
#include <stdlib.h>
#include <inc/cgc.h>
#include <inc/syntree.h>
#include <inc/util.h>

struct cgc_context {
	FILE *fp;
	int indent;
};

static void cgc_translation_unit(struct cgc_context *ctx, struct translation_unit *trans_unit);
static void cgc_external_declaration(struct cgc_context *ctx, struct external_declaration *external_decl);

struct cgc_context *cgc_context_init(FILE *fp, int indent) {
	struct cgc_context *ctx = mallocz(sizeof(*ctx));
	ctx->fp = fp;
	ctx->indent = indent;
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
	panic("ni");
}
