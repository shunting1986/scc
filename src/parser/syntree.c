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

struct multiplicative_expression *multiplicative_expression_init(struct cast_expression *cast_expr) {
	struct multiplicative_expression *multi_expr = mallocz(sizeof(*multi_expr));
	multi_expr->nodeType = MULTIPLICATIVE_EXPRESSION;
	multi_expr->oplist = dynarr_init();
	multi_expr->cast_expr_list = dynarr_init();
	dynarr_add(multi_expr->cast_expr_list, cast_expr);
	return multi_expr;
}

struct additive_expression *additive_expression_init(struct multiplicative_expression *mul_expr) {
	struct additive_expression *add_expr = mallocz(sizeof(*add_expr));
	add_expr->nodeType = ADDITIVE_EXPRESSION;
	add_expr->oplist = dynarr_init();
	add_expr->mul_expr_list = dynarr_init();
	dynarr_add(add_expr->mul_expr_list, mul_expr);
	return add_expr;
}

struct shift_expression *shift_expression_init(struct additive_expression *add_expr) {
	struct shift_expression *shift_expr = mallocz(sizeof(*shift_expr));
	shift_expr->nodeType = SHIFT_EXPRESSION;
	shift_expr->oplist = dynarr_init();
	shift_expr->add_expr_list = dynarr_init();
	dynarr_add(shift_expr->add_expr_list, add_expr);
	return shift_expr;
}

struct relational_expression *relational_expression_init(struct shift_expression *shift_expr) {
	struct relational_expression *rel_expr = mallocz(sizeof(*rel_expr));
	rel_expr->nodeType = RELATIONAL_EXPRESSION;
	rel_expr->oplist = dynarr_init();
	rel_expr->shift_expr_list = dynarr_init();
	dynarr_add(rel_expr->shift_expr_list, shift_expr);
	return rel_expr;
}

struct equality_expression *equality_expression_init(struct relational_expression *rel_expr) {
	struct equality_expression *eq_expr = mallocz(sizeof(*eq_expr));
	eq_expr->nodeType = EQUALITY_EXPRESSION;
	eq_expr->oplist = dynarr_init();
	eq_expr->rel_expr_list = dynarr_init();
	dynarr_add(eq_expr->rel_expr_list, rel_expr);
	return eq_expr;
}

struct and_expression *and_expression_init(struct equality_expression *eq_expr) {
	struct and_expression *and_expr = mallocz(sizeof(*and_expr));
	and_expr->nodeType = AND_EXPRESSION;
	and_expr->eq_expr_list = dynarr_init();
	dynarr_add(and_expr->eq_expr_list, eq_expr);
	return and_expr;
}

struct exclusive_or_expression *exclusive_or_expression_init(struct and_expression *and_expr) {
	struct exclusive_or_expression *xor_expr = mallocz(sizeof(*xor_expr));
	xor_expr->nodeType = EXCLUSIVE_OR_EXPRESSION;
	xor_expr->and_expr_list = dynarr_init();
	dynarr_add(xor_expr->and_expr_list, and_expr);
	return xor_expr;
}

struct inclusive_or_expression *inclusive_or_expression_init(struct exclusive_or_expression *xor_expr) {
	struct inclusive_or_expression *or_expr = mallocz(sizeof(*or_expr));
	or_expr->nodeType = INCLUSIVE_OR_EXPRESSION;
	or_expr->xor_expr_list = dynarr_init();
	dynarr_add(or_expr->xor_expr_list, xor_expr);
	return or_expr;
}
