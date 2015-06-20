#ifndef _INC_SYNTREE_NODE_H
#define _INC_SYNTREE_NODE_H

#ifdef __cplusplus
extern "C" {
#endif

enum syntree_node_type {
#define DEF(t) t,
#include <parser/node-type.def>
#undef DEF
	MAX_NODE_TYPE,
};

const char *node_type_str(unsigned int node_type);

struct dynarr;

struct declarator;
struct relational_expression;
struct shift_expression;
struct additive_expression;
struct multiplicative_expression;
struct cast_expression;

#define constant_expression conditional_expression

struct syntreebasenode {
	int nodeType;
};

struct translation_unit {
	int nodeType;
	struct dynarr *external_decl_list;
};

struct translation_unit *translation_unit_init(void);

struct external_declaration {
	int nodeType;
	struct declaration_specifiers *decl_specifiers;
	
	// for function declaration
	struct declarator *func_def_declarator;
	struct compound_statement *compound_stmt;

	// for declaration
	struct init_declarator_list *init_declarator_list;
};

// further information need to be set for the returned external_declaration
struct external_declaration *external_declaration_init(struct declaration_specifiers *decl_specifiers);

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

struct parameter_declaration {
	int nodeType;
	struct declaration_specifiers *decl_specifiers;
	struct declarator *declarator;
};

struct parameter_declaration *parameter_declaration_init(struct declaration_specifiers *decl_specifiers, struct declarator *declarator);

struct parameter_type_list {
	int nodeType;
	int has_ellipsis;
	struct dynarr *param_decl_list; // not empty
};

struct parameter_type_list *parameter_type_list_init(void);

struct direct_declarator_suffix {
	int empty_bracket;
	int empty_paren;
	struct constant_expression *const_expr;
	struct parameter_type_list *param_type_list;
};

/* 
 * TODO handle abstract declarator here
 */
struct direct_declarator {
	int nodeType;

	char *id;
	struct declarator *declarator;

	struct dynarr *suff_list;
};

struct direct_declarator *direct_declarator_init();

struct type_qualifier_list {
	int nodeType;
	struct dynarr *darr; // each element is a type_qualifier (int)
};

struct type_qualifier_list *type_qualifier_list_init(void);

struct declarator {
	int nodeType;

	// ptr_list will not be NULL; each element if exist will not be null and its type
	// is type_qualifier_list
	struct dynarr *ptr_list;
	struct direct_declarator *direct_declarator;
};

struct declarator *declarator_init();

struct type_qualifier {
	int nodeType;
	int tok_tag;
};

struct type_qualifier *type_qualifier_init(int tok_tag);

struct storage_class_specifier {
	int nodeType;
	int tok_tag;
};

struct storage_class_specifier *storage_class_specifier_init(int tok_tag);

struct type_specifier {
	int nodeType;
	int tok_tag; // specify type
	union syntreenode *extra;
};

struct type_specifier *type_specifier_init(int tok_tag);

struct jump_statement {
	int nodeType;
	int init_tok_tag;
	char *goto_label;
	struct expression *ret_expr;
};

struct jump_statement *jump_statement_init(int init_tok_tag);

struct compound_statement {
	int nodeType;
	struct dynarr *declList;
	struct dynarr *stmtList;
};

struct compound_statement *compound_statement_init(struct dynarr *declList, struct dynarr *stmtList);

struct equality_expression {
	int nodeType;
	struct dynarr *oplist;
	struct dynarr *rel_expr_list;
};

struct equality_expression *equality_expression_init(struct relational_expression *rel_expr);
void equality_expression_destroy(struct equality_expression *eq_expr);

struct and_expression {
	int nodeType;
	struct dynarr *eq_expr_list;
};

struct and_expression *and_expression_init(struct equality_expression *eq_expr);
void and_expression_destroy(struct and_expression *and_expr);

struct exclusive_or_expression {
	int nodeType;
	struct dynarr *and_expr_list;
};

struct exclusive_or_expression *exclusive_or_expression_init(struct and_expression *and_expr);
void exclusive_or_expression_destroy(struct exclusive_or_expression *xor_expr);

struct inclusive_or_expression {
	int nodeType;
	struct dynarr *xor_expr_list;
};

struct inclusive_or_expression *inclusive_or_expression_init(struct exclusive_or_expression *xor_expr);
void inclusive_or_expression_destroy(struct inclusive_or_expression *or_expr);

struct logical_and_expression {
	int nodeType;
	struct dynarr *or_expr_list;
};

struct logical_and_expression *logical_and_expression_init(struct inclusive_or_expression *or_expr);
void logical_and_expression_destroy(struct logical_and_expression *and_expr);

struct logical_or_expression {
	int nodeType;
	struct dynarr *and_expr_list;
};

struct logical_or_expression *logical_or_expression_init(struct logical_and_expression *and_expr);
void logical_or_expression_destroy(struct logical_or_expression *lor_expr);

struct conditional_expression {
	int nodeType;
	struct dynarr *or_expr_list;
	struct dynarr *inner_expr_list;
};

struct conditional_expression *conditional_expression_init(struct logical_or_expression *or_expr);
// This is a shadow destroy: only destroy the thing allocated in the init method
void conditional_expression_destroy(struct conditional_expression *cond_expr);

struct relational_expression {
	int nodeType;
	struct dynarr *oplist;
	struct dynarr *shift_expr_list;
};

struct relational_expression *relational_expression_init(struct shift_expression *shift_expr);
void relational_expression_destroy(struct relational_expression *rel_expr);

struct shift_expression {
	int nodeType;
	struct dynarr *oplist;
	struct dynarr *add_expr_list;
};

struct shift_expression *shift_expression_init(struct additive_expression *add_expr);
void shift_expression_destroy(struct shift_expression *shift_expr);

struct additive_expression {
	int nodeType;
	struct dynarr *oplist;
	struct dynarr *mul_expr_list;
};

struct additive_expression *additive_expression_init(struct multiplicative_expression *mul_expr);
void additive_expression_destroy(struct additive_expression *add_expr);

struct multiplicative_expression {
	int nodeType;
	struct dynarr *oplist;
	struct dynarr *cast_expr_list;
};

struct multiplicative_expression *multiplicative_expression_init(struct cast_expression *cast_expr);
void multiplicative_expression_destroy(struct multiplicative_expression *mul_expr);

struct primary_expression {
	int nodeType;
	char *id;
	char *str;
	union token const_val_tok;
	struct expression *expr;
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
	struct unary_expression *inc_unary;
	struct unary_expression *dec_unary;

	// unary_operator cast_expression
	int unary_op;
	struct cast_expression *unary_op_cast;

	struct postfix_expression *postfix_expr;
};
struct unary_expression *unary_expression_init(void);

struct cast_expression {
	int nodeType;
	struct unary_expression *unary_expr;
	// TODO not complete yet
};

struct cast_expression *cast_expression_init();
void cast_expression_destroy(struct cast_expression *cast_expr);

struct assignment_expression {
	int nodeType;
	struct dynarr *unary_expr_list;
	struct dynarr *oplist;
	struct conditional_expression *cond_expr;
};

struct assignment_expression *assignment_expression_init();

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

#ifdef __cplusplus
}
#endif

#endif
