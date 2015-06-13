#include <stdio.h>
#include <inc/cgc.h>
#include <inc/syntree.h>
#include <inc/util.h>

struct cgc_context {
	FILE *fp;
	int indent;
};

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
	panic("ni");
}

