#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <inc/cgc.h>
#include <inc/syntree.h>
#include <inc/util.h>

struct cgc_context {
	FILE *fp;
	int indent;
	int step; // the step for indent
};

static void cgc_expression(struct cgc_context *ctx, struct expression *expr);
static void cgc_translation_unit(struct cgc_context *ctx, struct translation_unit *trans_unit);
static void cgc_external_declaration(struct cgc_context *ctx, struct external_declaration *external_decl);
static void cgc_function_definition(struct cgc_context *ctx, struct declaration_specifiers *decl_specifiers, struct declarator *func_def_declarator, struct compound_statement *compound_stmt);
static void cgc_declaration(struct cgc_context *ctx, struct declaration_specifiers *decl_specifiers, struct init_declarator_list *init_declarator_list);
static void cgc_declaration_specifiers(struct cgc_context *ctx, struct declaration_specifiers *decl_specifiers);
static void cgc_type_specifier(struct cgc_context *ctx, struct type_specifier *type_specifier);
static void cgc_declarator(struct cgc_context *ctx, struct declarator *declarator);
static void cgc_direct_declarator(struct cgc_context *ctx, struct direct_declarator *direct_declarator);
static void cgc_compound_statement(struct cgc_context *ctx, struct compound_statement *compound_stmt);
static void cgc_statement(struct cgc_context *ctx, struct syntreebasenode *stmt);
static void cgc_init_declarator_list(struct cgc_context *ctx, struct init_declarator_list *init_declarator_list);
static void cgc_init_declarator(struct cgc_context *ctx, struct init_declarator *init_declarator);

#define CGC_BINARY_OP_EXPR(ctx, container_expr, subexpr_list, subexpr_type, oplist, single_op) do { \
	cgc_ ## subexpr_type(ctx, dynarr_get(subexpr_list, 0)); \
	int i; \
	for (i = 1; i < dynarr_size(subexpr_list); i++) { \
		int tok_op = oplist != NULL ? (int) (long) dynarr_get(oplist, i - 1) : single_op; \
		cgc_print(ctx, " %s ", cgc_get_op_str(tok_op)); \
		cgc_ ## subexpr_type(ctx, dynarr_get(subexpr_list, i)); \
	} \
} while (0)

// this method will take care of the indent
static void cgc_printi(struct cgc_context *ctx, const char *fmt, ...) {
	va_list va;
	fprintf(ctx->fp, "%*s", ctx->indent, "");
	va_start(va, fmt);
	vfprintf(ctx->fp, fmt, va);
	va_end(va);
}

// this method does not add indent
static void cgc_print(struct cgc_context *ctx, const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	vfprintf(ctx->fp, fmt, va);
	va_end(va);
}

static void cgc_indent(struct cgc_context *ctx) {
	fprintf(ctx->fp, "%*s", ctx->indent, "");
}

struct cgc_context *cgc_context_init(FILE *fp, int indent) {
	struct cgc_context *ctx = mallocz(sizeof(*ctx));
	ctx->fp = fp;
	ctx->indent = indent;
	ctx->step = 2;
	return ctx;
}

void cgc_context_destroy(struct cgc_context *ctx) {
	free(ctx);
}

void cgc_tree(struct cgc_context *ctx, struct syntree *tree) {
	cgc_translation_unit(ctx, tree->trans_unit);
}

static void cgc_translation_unit(struct cgc_context *ctx, struct translation_unit *trans_unit) {
	DYNARR_FOREACH_BEGIN(trans_unit->external_decl_list, external_declaration, each);
		cgc_external_declaration(ctx, each);
	DYNARR_FOREACH_END();
}

static void cgc_external_declaration(struct cgc_context *ctx, struct external_declaration *external_decl) {
	if (external_decl->func_def_declarator != NULL) {
		cgc_function_definition(ctx, external_decl->decl_specifiers, external_decl->func_def_declarator, external_decl->compound_stmt);
	} else {
		cgc_declaration(ctx, external_decl->decl_specifiers, external_decl->init_declarator_list);
	}
}

static void cgc_function_definition(struct cgc_context *ctx, struct declaration_specifiers* decl_specifiers, struct declarator *func_def_declarator, struct compound_statement *compound_stmt) {
	cgc_declaration_specifiers(ctx, decl_specifiers);
	cgc_print(ctx, " ");
	cgc_declarator(ctx, func_def_declarator);
	cgc_print(ctx, "\n");
	cgc_compound_statement(ctx, compound_stmt);
}

static void cgc_compound_statement(struct cgc_context *ctx, struct compound_statement *compound_stmt) {
	cgc_indent(ctx); cgc_print(ctx, "{\n");

	ctx->indent += ctx->step;
	DYNARR_FOREACH_BEGIN(compound_stmt->declList, declaration, each);
		cgc_declaration(ctx, each->decl_specifiers, each->init_declarator_list);
	DYNARR_FOREACH_END();

	DYNARR_FOREACH_BEGIN(compound_stmt->stmtList, syntreebasenode, each);
		cgc_statement(ctx, each);
	DYNARR_FOREACH_END();

	ctx->indent -= ctx->step;
	cgc_indent(ctx); cgc_print(ctx, "}\n");
}

static void cgc_unary_expression(struct cgc_context *ctx, struct unary_expression *unary_expr) {
	panic("ni");
}

static void cgc_cast_expression(struct cgc_context *ctx, struct cast_expression *expr) {
	panic("ni");
}

static void cgc_multiplicative_expression(struct cgc_context *ctx, struct multiplicative_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->cast_expr_list, cast_expression, expr->oplist, TOK_UNDEF);
}

static void cgc_additive_expression(struct cgc_context *ctx, struct additive_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->mul_expr_list, multiplicative_expression, expr->oplist, TOK_UNDEF);
}

static void cgc_shift_expression(struct cgc_context *ctx, struct shift_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->add_expr_list, additive_expression, expr->oplist, TOK_UNDEF);
}

static void cgc_relational_expression(struct cgc_context *ctx, struct relational_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->shift_expr_list, shift_expression, expr->oplist, TOK_UNDEF);
}

static void cgc_equality_expression(struct cgc_context *ctx, struct equality_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->rel_expr_list, relational_expression, expr->oplist, TOK_UNDEF);
}

static void cgc_and_expression(struct cgc_context *ctx, struct and_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->eq_expr_list, equality_expression, NULL, TOK_AMPERSAND);
}

static void cgc_exclusive_or_expression(struct cgc_context *ctx, struct exclusive_or_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->and_expr_list, and_expression, NULL, TOK_XOR);
}

static void cgc_inclusive_or_expression(struct cgc_context *ctx, struct inclusive_or_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->xor_expr_list, exclusive_or_expression, NULL, TOK_VERT_BAR);
}

static void cgc_logical_and_expression(struct cgc_context *ctx, struct logical_and_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->or_expr_list, inclusive_or_expression, NULL, TOK_LOGIC_AND);
}

static void cgc_logical_or_expression(struct cgc_context *ctx, struct logical_or_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->and_expr_list, logical_and_expression, NULL, TOK_LOGIC_OR);
}

static void cgc_conditional_expression(struct cgc_context *ctx, struct conditional_expression *cond_expr) {
	int i;
	cgc_logical_or_expression(ctx, dynarr_get(cond_expr->or_expr_list, 0));

	// XXX can do better formatting when multiple '?' chained together
	for (i = 1; i < dynarr_size(cond_expr->or_expr_list); i++) {
		struct logical_or_expression *or_expr = dynarr_get(cond_expr->or_expr_list, i);
		struct expression *expr = dynarr_get(cond_expr->inner_expr_list, i - 1);
		cgc_print(ctx, " ? ");
		cgc_expression(ctx, expr);
		cgc_print(ctx, " : ");
		cgc_logical_or_expression(ctx, or_expr);
	}
}

static void cgc_assignment_expression(struct cgc_context *ctx, struct assignment_expression *expr) {
	int i;
	for (i = 0; i < dynarr_size(expr->unary_expr_list); i++) {
		struct unary_expression *unary_expr = dynarr_get(expr->unary_expr_list, i);
		int optok = (int)(long) dynarr_get(expr->oplist, i);
		cgc_unary_expression(ctx, unary_expr);
		cgc_print(ctx, " ");
		cgc_get_op_str(optok);
		cgc_print(ctx, " ");
	}
	cgc_conditional_expression(ctx, expr->cond_expr);
}

static void cgc_expression(struct cgc_context *ctx, struct expression *expr) {
	int first = 1;
	DYNARR_FOREACH_BEGIN(expr->darr, assignment_expression, each);
		if (!first) {
			cgc_print(ctx, ", ");
		}
		first = 0;
		cgc_assignment_expression(ctx, each);
	DYNARR_FOREACH_END();
}

static void cgc_expression_statement(struct cgc_context *ctx, struct expression_statement *stmt) {
	cgc_indent(ctx); 
	cgc_expression(ctx, stmt->expr);
	cgc_print(ctx, "\n");
}

static void cgc_statement(struct cgc_context *ctx, struct syntreebasenode *stmt) {
	switch (stmt->nodeType) {
	case EXPRESSION_STATEMENT:
		cgc_expression_statement(ctx, (struct expression_statement *) stmt);
		break;
	default:
		panic("unexpected node type %s", node_type_str(stmt->nodeType));
	}
}

static void cgc_declarator(struct cgc_context *ctx, struct declarator *declarator) {
	cgc_direct_declarator(ctx, declarator->direct_declarator);
}

static void cgc_direct_declarator(struct cgc_context *ctx, struct direct_declarator *direct_declarator) {
	assert(direct_declarator->id != NULL);
	cgc_print(ctx, "%s", direct_declarator->id);
	// TODO
}

static void cgc_declaration(struct cgc_context *ctx, struct declaration_specifiers *decl_specifiers, struct init_declarator_list *init_declarator_list) {
	cgc_indent(ctx);
	cgc_declaration_specifiers(ctx, decl_specifiers);
	cgc_print(ctx, " ");
	cgc_init_declarator_list(ctx, init_declarator_list);
	cgc_print(ctx, ";\n");
}

static void cgc_init_declarator_list(struct cgc_context *ctx, struct init_declarator_list *init_declarator_list) {
	int is_first = 1;
	DYNARR_FOREACH_BEGIN(init_declarator_list->darr, init_declarator, each);
		if (!is_first) {
			cgc_print(ctx, ", ");
		}
		is_first = 0;
		cgc_init_declarator(ctx, each);
	DYNARR_FOREACH_END();
}

static void cgc_init_declarator(struct cgc_context *ctx, struct init_declarator *init_declarator) {
	cgc_declarator(ctx, init_declarator->declarator);
	assert(init_declarator->initializer == NULL); // not supported yet
}

static void cgc_declaration_specifiers(struct cgc_context *ctx, struct declaration_specifiers *decl_specifiers) {
	cgc_indent(ctx);
	DYNARR_FOREACH_BEGIN(decl_specifiers->darr, syntreebasenode, each);
		switch (each->nodeType) {
		case TYPE_SPECIFIER:
			cgc_type_specifier(ctx, (struct type_specifier *) each);
			break;
		default:
			panic("ni");
		}
	DYNARR_FOREACH_END();
}

static void cgc_type_specifier(struct cgc_context *ctx, struct type_specifier *type_specifier) {
	switch (type_specifier->tok_tag) {
	case TOK_INT:
		cgc_print(ctx, "int");
		break;
	default:
		panic("ni");
	}
}

