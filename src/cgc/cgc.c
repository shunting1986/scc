#include <stdio.h>
#include <stdlib.h>
#include <inc/cgc.h>
#include <inc/syntree.h>
#include <inc/util.h>

struct cgc_context {
	FILE *fp;
	int indent;
};

static void cgc_node(struct cgc_context *ctx, struct syntreebasenode *node);

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
	cgc_node(ctx, (struct syntreebasenode *) tree->trans_unit);
}

static void cgc_node(struct cgc_context *ctx, struct syntreebasenode *node) {
	switch (node->nodeType) {
	default:
		panic("need implement: %s", node_type_str(node->nodeType));
	}
}

