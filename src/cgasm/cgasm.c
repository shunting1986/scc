#include <inc/cgasm.h>
#include <inc/util.h>
#include <inc/syntree.h>
#include <inc/syntree-node.h>

struct cgasm_context {
	FILE *fp;
};

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

static void cgasm_translation_unit(struct cgasm_context *ctx, struct translation_unit *trans_unit) {
	panic("ni");
}
