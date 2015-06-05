#include <inc/syntree.h>
#include <inc/util.h>

void syntree_dump(struct syntree *tree) {
	panic("syntree_dump ni"); // TODO
}

// TODO add sth like syntree_free

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

struct direct_declarator *direct_declarator_init() {
	struct direct_declarator *dd = malloc(sizeof(*dd));
	dd->nodeType = DIRECT_DECLARATOR;
	return dd;
}

struct declarator *declarator_init() {
	struct declarator *declarator = calloc(1, sizeof(*declarator));
	declarator->nodeType = DECLARATOR;
	return declarator;
}

struct compound_statement *compound_statement_init(struct dynarr *declList, struct dynarr *stmtList) {
	struct compound_statement *stmt = mallocz(sizeof(*stmt));
	stmt->nodeType = COMPOUND_STATEMENT;
	stmt->declList = declList;
	stmt->stmtList = stmtList;
	return stmt;
}
