#include <assert.h>
#include <inc/syntree.h>
#include <inc/util.h>
#include <inc/dynarr.h>

void syntree_dump(struct syntree *tree) {
	panic("syntree_dump ni"); // TODO
}

// TODO add sth like syntree_free

struct declaration_specifiers *declaration_specifiers_init(struct dynarr *darr) {
	struct declaration_specifiers *specs = malloc(sizeof(*specs));
	specs->nodeType = DECLARATION_SPECIFIERS;
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

struct init_declarator_list *init_declarator_list_init(struct dynarr *darr) {
	struct init_declarator_list *declarator_list = mallocz(sizeof(*declarator_list));
	declarator_list->nodeType = INIT_DECLARATOR_LIST;
	declarator_list->darr = darr;
	return declarator_list;
}

struct init_declarator *init_declarator_init(struct declarator *declarator, struct initializer *initializer) {
	struct init_declarator *init_declarator = mallocz(sizeof(*init_declarator));
	init_declarator->nodeType = INIT_DECLARATOR;
	init_declarator->declarator = declarator;
	init_declarator->initializer = initializer;
	return init_declarator;
}

struct declaration *declaration_init(struct declaration_specifiers *decl_specifiers, struct init_declarator_list *init_declarator_list) {
	struct declaration *decl = mallocz(sizeof(*decl));
	decl->nodeType = DECLARATION;
	decl->decl_specifiers = decl_specifiers;
	decl->init_declarator_list = init_declarator_list;
	return decl;
}

struct expression_statement *expression_statement_init(struct expression *expr) {
	struct expression_statement *stmt = mallocz(sizeof(*stmt));
	stmt->nodeType = EXPRESSION_STATEMENT;
	stmt->expr = expr;
	return stmt;
}

struct expression *expression_init(struct dynarr *darr) {
	struct expression *expr = mallocz(sizeof(*expr));
	assert(dynarr_size(darr) > 0);
	expr->nodeType = EXPRESSION;
	expr->darr = darr;
	return expr;
}

struct cast_expression *cast_expression_init() {
	struct cast_expression *expr = mallocz(sizeof(*expr));
	expr->nodeType = CAST_EXPRESSION;
	return expr;
}

struct primary_expression *primary_expression_init() {
	struct primary_expression *expr = mallocz(sizeof(*expr));
	expr->nodeType = PRIMARY_EXPRESSION;
	return expr;
}

struct postfix_expression *postfix_expression_init(struct primary_expression *prim_expr) {
	struct postfix_expression *post_expr = mallocz(sizeof(*post_expr));
	post_expr->nodeType = POSTFIX_EXPRESSION;
	post_expr->prim_expr = prim_expr;
	post_expr->suff_list = dynarr_init();
	return post_expr;
}

struct argument_expression_list *argument_expression_list_init() {
	struct argument_expression_list *arg_list = mallocz(sizeof(*arg_list));
	arg_list->nodeType = ARGUMENT_EXPRESSION_LIST;
	arg_list->list = dynarr_init();
	return arg_list;
}

struct unary_expression *unary_expression_init() {
	struct unary_expression *unary_expr = mallocz(sizeof(*unary_expr));
	unary_expr->nodeType = UNARY_EXPRESSION;
	return unary_expr;
}
