#ifndef _INC_SYNTREE_H
#define _INC_SYNTREE_H

#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct syntree;
struct dynarr;

struct declarator;

void syntree_dump(struct syntree *tree);

enum syntree_node_type {
	CAST_EXPRESSION,
	DECLARATION_SPECIFIERS,
	DECLARATION,
	TRANSLATION_UNIT,
	EXTERNAL_DECL,
	EXPRESSION_STATEMENT,
	EXPRESSION,
	TYPE_SPECIFIER,
	DIRECT_DECLARATOR,
	DECLARATOR,
	COMPOUND_STATEMENT,
	INIT_DECLARATOR_LIST,
	INIT_DECLARATOR,
	INITIALIZER,
	PRIMARY_EXPRESSION,
	POSTFIX_EXPRESSION,
	ARGUMENT_EXPRESSION_LIST,
	UNARY_EXPRESSION,
	MULTIPLICATIVE_EXPRESSION,
	ADDITIVE_EXPRESSION,
	SHIFT_EXPRESSION,
};

struct syntreebasenode {
	int nodeType;
};

struct translation_unit_node {
	int nodeType;
	struct dynarr *external_decl_list;
};

struct external_decl_node {
	int nodeType;
};

struct initializer {
	int nodeType;
};

struct init_declarator {
	int nodeType;
	struct declarator *declarator;
	struct initializer *initializer;
};

struct init_declarator *init_declarator_init(struct declarator *declarator, struct initializer *initializer);


struct init_declarator_list {
	int nodeType;
	struct dynarr *darr;
};

struct init_declarator_list *init_declarator_list_init(struct dynarr *darr);

struct declaration {
	int nodeType;
	struct declaration_specifiers *decl_specifiers;
	struct init_declarator_list *init_declarator_list;
};

struct declaration *declaration_init(struct declaration_specifiers *decl_specifiers, struct init_declarator_list *init_declarator_list);

struct declaration_specifiers {
	int nodeType;
	struct dynarr *darr;
};

struct declaration_specifiers *declaration_specifiers_init(struct dynarr *darr);

struct direct_declarator {
	int nodeType;
	char *id;
};

struct direct_declarator *direct_declarator_init();

struct declarator {
	int nodeType;
	struct direct_declarator *directDeclarator;
};

struct declarator *declarator_init();

struct type_specifier {
	int nodeType;
	int tok_tag; // specify type
	union syntreenode *extra;
};

struct type_specifier *type_specifier_init(int tok_tag);

struct compound_statement {
	int nodeType;
	struct dynarr *declList;
	struct dynarr *stmtList;
};

struct compound_statement *compound_statement_init(struct dynarr *declList, struct dynarr *stmtList);

struct shift_expression {
	int nodeType;
	struct additive_expression *first_expr;
	struct dynarr *oplist;
	struct dynarr *add_expr_list;
};

struct shift_expression *shift_expression_init(struct additive_expression *add_expr);

struct additive_expression {
	int nodeType;
	struct multiplicative_expression *first_expr;
	struct dynarr *oplist;
	struct dynarr *mul_expr_list;
};

struct additive_expression *additive_expression_init(struct multiplicative_expression *mul_expr);

struct multiplicative_expression {
	int nodeType;
	struct cast_expression *first_expr;
	struct dynarr *oplist;
	struct dynarr *cast_expr_list;
};

struct multiplicative_expression *multiplicative_expression_init(struct cast_expression *cast_expr);

struct primary_expression {
	int nodeType;
	char *id;
	char *str;
};

struct primary_expression *primary_expression_init();

struct argument_expression_list {
	int nodeType;
	struct dynarr *list;
};

struct argument_expression_list *argument_expression_list_init();

struct postfix_expression_suffix {
	struct expression *ind;
	struct argument_expression_list *arg_list; // contains an empty list if the argument list is empty
		// be NULL if the suffix is in other form
	char *dot_id;
	char *ptr_id;
	int is_inc;
	int is_dec;
};

struct postfix_expression {
	int nodeType;
	struct primary_expression *prim_expr;
	struct dynarr *suff_list;
};

struct postfix_expression *postfix_expression_init(struct primary_expression *prim_expr);

struct unary_expression {
	int nodeType;
	struct postfix_expression *postfix_expr;
};
struct unary_expression *unary_expression_init();

struct cast_expression {
	int nodeType;
	struct unary_expression *unary_expr;
};

struct cast_expression *cast_expression_init();

struct assignment_expression {
	int nodeType;
};

struct expression {
	int nodeType;
	struct dynarr *darr; // list of assignment expressions, evaluate from left to right
};

struct expression *expression_init(struct dynarr *darr);

struct expression_statement {
	int nodeType;
	struct expression *expr;
};

struct expression_statement *expression_statement_init(struct expression *expr);

// I decide to put the syntreenode definition in .h file.
// May revise to put in .c file later
union syntreenode {
	struct syntreebasenode base;
	struct translation_unit_node translation_unit;
	struct external_decl_node external_decl;
};

#ifdef __cplusplus
}
#endif

#endif

