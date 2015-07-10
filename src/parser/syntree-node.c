#include <assert.h>
#include <stdio.h>
#include <inc/syntree.h>
#include <inc/util.h>
#include <inc/dynarr.h>
#include <inc/lexer.h>
#include <inc/cgc.h>

// get string form of the syntax node type
static const char *node_type_str_table[] = {
#define DEF(t) [t] = #t,
#include <parser/node-type.def>
#undef DEF
};

const char *node_type_str(unsigned int node_type) {
	if (node_type >= MAX_NODE_TYPE) {
		panic("invalid node type %d", node_type);
	}
	return node_type_str_table[node_type];
}

// following are node constructor and destructor definitions

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
	dd->suff_list = dynarr_init();
	return dd;
}

struct declarator *declarator_init() {
	struct declarator *declarator = calloc(1, sizeof(*declarator));
	declarator->nodeType = DECLARATOR;
	return declarator;
}

struct jump_statement *jump_statement_init(int init_tok_tag) {
	struct jump_statement *stmt = mallocz(sizeof(*stmt));
	stmt->nodeType = JUMP_STATEMENT;
	stmt->init_tok_tag = init_tok_tag;
	return stmt;
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

void cast_expression_destroy(struct cast_expression *cast_expr) {
	free(cast_expr);
}

struct primary_expression *primary_expression_init() {
	struct primary_expression *expr = mallocz(sizeof(*expr));
	expr->nodeType = PRIMARY_EXPRESSION;
	expr->const_val_tok.tok_tag = TOK_UNDEF;
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

struct unary_expression *unary_expression_init(void) {
	struct unary_expression *unary_expr = mallocz(sizeof(*unary_expr));
	unary_expr->nodeType = UNARY_EXPRESSION;
	unary_expr->unary_op = TOK_UNDEF;
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

void multiplicative_expression_destroy(struct multiplicative_expression *mul_expr) {
	dynarr_destroy(mul_expr->cast_expr_list);
	dynarr_destroy(mul_expr->oplist);
	free(mul_expr);
}

struct additive_expression *additive_expression_init(struct multiplicative_expression *mul_expr) {
	struct additive_expression *add_expr = mallocz(sizeof(*add_expr));
	add_expr->nodeType = ADDITIVE_EXPRESSION;
	add_expr->oplist = dynarr_init();
	add_expr->mul_expr_list = dynarr_init();
	dynarr_add(add_expr->mul_expr_list, mul_expr);
	return add_expr;
}

void additive_expression_destroy(struct additive_expression *add_expr) {
	dynarr_destroy(add_expr->mul_expr_list);
	dynarr_destroy(add_expr->oplist);
	free(add_expr);
}

struct shift_expression *shift_expression_init(struct additive_expression *add_expr) {
	struct shift_expression *shift_expr = mallocz(sizeof(*shift_expr));
	shift_expr->nodeType = SHIFT_EXPRESSION;
	shift_expr->oplist = dynarr_init();
	shift_expr->add_expr_list = dynarr_init();
	dynarr_add(shift_expr->add_expr_list, add_expr);
	return shift_expr;
}

void shift_expression_destroy(struct shift_expression *shift_expr) {
	dynarr_destroy(shift_expr->add_expr_list);
	dynarr_destroy(shift_expr->oplist);
	free(shift_expr);
}

struct relational_expression *relational_expression_init(struct shift_expression *shift_expr) {
	struct relational_expression *rel_expr = mallocz(sizeof(*rel_expr));
	rel_expr->nodeType = RELATIONAL_EXPRESSION;
	rel_expr->oplist = dynarr_init();
	rel_expr->shift_expr_list = dynarr_init();
	dynarr_add(rel_expr->shift_expr_list, shift_expr);
	return rel_expr;
}

void relational_expression_destroy(struct relational_expression *rel_expr) {
	dynarr_destroy(rel_expr->shift_expr_list);
	dynarr_destroy(rel_expr->oplist);
	free(rel_expr);
}

struct equality_expression *equality_expression_init(struct relational_expression *rel_expr) {
	struct equality_expression *eq_expr = mallocz(sizeof(*eq_expr));
	eq_expr->nodeType = EQUALITY_EXPRESSION;
	eq_expr->oplist = dynarr_init();
	eq_expr->rel_expr_list = dynarr_init();
	dynarr_add(eq_expr->rel_expr_list, rel_expr);
	return eq_expr;
}

void equality_expression_destroy(struct equality_expression *eq_expr) {
	dynarr_destroy(eq_expr->rel_expr_list);
	dynarr_destroy(eq_expr->oplist);
	free(eq_expr);
}

struct and_expression *and_expression_init(struct equality_expression *eq_expr) {
	struct and_expression *and_expr = mallocz(sizeof(*and_expr));
	and_expr->nodeType = AND_EXPRESSION;
	and_expr->eq_expr_list = dynarr_init();
	dynarr_add(and_expr->eq_expr_list, eq_expr);
	return and_expr;
}

void and_expression_destroy(struct and_expression *and_expr) {
	dynarr_destroy(and_expr->eq_expr_list);
	free(and_expr);
}

struct exclusive_or_expression *exclusive_or_expression_init(struct and_expression *and_expr) {
	struct exclusive_or_expression *xor_expr = mallocz(sizeof(*xor_expr));
	xor_expr->nodeType = EXCLUSIVE_OR_EXPRESSION;
	xor_expr->and_expr_list = dynarr_init();
	dynarr_add(xor_expr->and_expr_list, and_expr);
	return xor_expr;
}

void exclusive_or_expression_destroy(struct exclusive_or_expression *xor_expr) {
	dynarr_destroy(xor_expr->and_expr_list);
	free(xor_expr);
}

struct inclusive_or_expression *inclusive_or_expression_init(struct exclusive_or_expression *xor_expr) {
	struct inclusive_or_expression *or_expr = mallocz(sizeof(*or_expr));
	or_expr->nodeType = INCLUSIVE_OR_EXPRESSION;
	or_expr->xor_expr_list = dynarr_init();
	dynarr_add(or_expr->xor_expr_list, xor_expr);
	return or_expr;
}

void inclusive_or_expression_destroy(struct inclusive_or_expression *or_expr) {
	dynarr_destroy(or_expr->xor_expr_list);
	free(or_expr);
}

struct logical_and_expression *logical_and_expression_init(struct inclusive_or_expression *or_expr) {
	struct logical_and_expression *logic_and_expr = mallocz(sizeof(*logic_and_expr));
	logic_and_expr->nodeType = LOGICAL_AND_EXPRESSION;
	logic_and_expr->or_expr_list = dynarr_init();
	dynarr_add(logic_and_expr->or_expr_list, or_expr);
	return logic_and_expr;
}

void logical_and_expression_destroy(struct logical_and_expression *and_expr) {
	dynarr_destroy(and_expr->or_expr_list);
	free(and_expr);
}

struct logical_or_expression *logical_or_expression_init(struct logical_and_expression *and_expr) {
	struct logical_or_expression *or_expr = mallocz(sizeof(*or_expr));
	or_expr->nodeType = LOGICAL_OR_EXPRESSION;
	or_expr->and_expr_list = dynarr_init();
	dynarr_add(or_expr->and_expr_list, and_expr);
	return or_expr;
}

void logical_or_expression_destroy(struct logical_or_expression *lor_expr) {
	dynarr_destroy(lor_expr->and_expr_list);
	free(lor_expr);
}

struct conditional_expression *conditional_expression_init(struct logical_or_expression *or_expr) {
	struct conditional_expression *cond_expr = mallocz(sizeof(*cond_expr));
	cond_expr->nodeType = CONDITIONAL_EXPRESSION;
	cond_expr->or_expr_list = dynarr_init();
	cond_expr->inner_expr_list = dynarr_init();
	dynarr_add(cond_expr->or_expr_list, or_expr);
	return cond_expr;
}

void conditional_expression_destroy(struct conditional_expression *cond_expr) {
	dynarr_destroy(cond_expr->inner_expr_list);
	dynarr_destroy(cond_expr->or_expr_list);
	free(cond_expr);
}

struct assignment_expression *assignment_expression_init() {
	struct assignment_expression *assign_expr = mallocz(sizeof(*assign_expr));
	assign_expr->nodeType = ASSIGNMENT_EXPRESSION;
	assign_expr->unary_expr_list = dynarr_init();
	assign_expr->oplist = dynarr_init();
	return assign_expr;
}

struct external_declaration *external_declaration_init(struct declaration_specifiers *decl_specifiers) {
	struct external_declaration *external_decl = mallocz(sizeof(*external_decl));
	external_decl->nodeType = EXTERNAL_DECLARATION;
	external_decl->decl_specifiers = decl_specifiers;
	return external_decl;
}

struct translation_unit *translation_unit_init(void) {
	struct translation_unit *tu = mallocz(sizeof(*tu));
	tu->nodeType = TRANSLATION_UNIT;
	tu->external_decl_list = dynarr_init();
	return tu;
}

struct type_qualifier_list *type_qualifier_list_init(void) {
	struct type_qualifier_list *nd = mallocz(sizeof(*nd));
	nd->nodeType = TYPE_QUALIFIER_LIST;
	nd->darr = dynarr_init();
	return nd;
}

struct parameter_type_list *parameter_type_list_init(void) {
	struct parameter_type_list *nd = mallocz(sizeof(*nd));
	nd->nodeType = PARAMETER_TYPE_LIST;
	nd->param_decl_list = dynarr_init();
	return nd;
}

struct parameter_declaration *parameter_declaration_init(struct declaration_specifiers *decl_specifiers, struct declarator *declarator) {
	struct parameter_declaration *decl = mallocz(sizeof(*decl));
	decl->nodeType = PARAMETER_DECLARATION;
	decl->decl_specifiers = decl_specifiers;
	decl->declarator = declarator;
	return decl;
}

struct type_qualifier *type_qualifier_init(int tok_tag) {
	struct type_qualifier *qual = mallocz(sizeof(*qual));
	qual->nodeType = TYPE_QUALIFIER;
	qual->tok_tag = tok_tag;
	return qual;
}

struct storage_class_specifier *storage_class_specifier_init(int tok_tag) {
	struct storage_class_specifier *sc = mallocz(sizeof(*sc));
	sc->nodeType = STORAGE_CLASS_SPECIFIER;
	sc->tok_tag = tok_tag;
	return sc;
}

struct iteration_statement *iteration_statement_init(int iterType) {
	struct iteration_statement *stmt = mallocz(sizeof(*stmt));
	stmt->nodeType = ITERATION_STATEMENT;
	stmt->iterType = iterType;
	return stmt;
}

