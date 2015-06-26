#include <assert.h>
#include <stdio.h>
#include <inc/syntree.h>
#include <inc/util.h>
#include <inc/dynarr.h>
#include <inc/lexer.h>
#include <inc/cgc.h>

struct syntree *syntree_init(struct translation_unit *trans_unit) {
	struct syntree *tree = mallocz(sizeof(*tree));
	tree->trans_unit = trans_unit;
	return tree;
}

void syntree_dump(struct syntree *tree) {
	struct cgc_context *ctx = cgc_context_init(stdout, 0);
	cgc_tree(ctx, tree);
	cgc_context_destroy(ctx);
}

void syntree_destroy(struct syntree *tree) {
	panic("ni");
}


