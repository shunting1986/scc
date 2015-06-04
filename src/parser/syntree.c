#include <inc/syntree.h>

void syntree_dump(struct syntree *tree) {
	panic("syntree_dump ni"); // TODO
}

struct declaration_specifiers *declaration_specifiers_init(struct dynarr *darr) {
	struct declaration_specifiers *specs = malloc(sizeof(*specs));
	specs->darr = darr;
	return specs;
}

struct type_specifier *type_specifier_init(int tok_tag) {
	struct type_specifier *sp = malloc(sizeof(*sp));
	sp->nodeType = TYPE_SPECIFIER;
	sp->tok_tag = tok_tag;
	sp->extra = NULL;
	return sp;
}


