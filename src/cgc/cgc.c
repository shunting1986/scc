#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <inc/cgc.h>
#include <inc/syntree.h>
#include <inc/util.h>
#include <inc/keyword.h>

struct cgc_context {
	FILE *fp;
	int indent;
	int step; // the step for indent
};

#define cgc_constant_expression cgc_conditional_expression

#define CGC_BINARY_OP_EXPR(ctx, container_expr, subexpr_list, subexpr_type, oplist, single_op) do { \
	cgc_ ## subexpr_type(ctx, dynarr_get(subexpr_list, 0)); \
	int i; \
	for (i = 1; i < dynarr_size(subexpr_list); i++) { \
		int tok_op = oplist != NULL ? (int) (long) dynarr_get(oplist, i - 1) : single_op; \
		cgc_print(ctx, " %s ", cgc_get_op_str(tok_op)); \
		cgc_ ## subexpr_type(ctx, dynarr_get(subexpr_list, i)); \
	} \
} while (0)

// this method does not add indent
void cgc_print(struct cgc_context *ctx, const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	vfprintf(ctx->fp, fmt, va);
	va_end(va);
}

void cgc_println(struct cgc_context *ctx, const char *fmt, ...) {
	va_list va;
	va_start(va, fmt);
	vfprintf(ctx->fp, fmt, va);
	va_end(va);
	fprintf(ctx->fp, "\n");
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

void cgc_translation_unit(struct cgc_context *ctx, struct translation_unit *trans_unit) {
	DYNARR_FOREACH_BEGIN(trans_unit->external_decl_list, external_declaration, each);
		cgc_external_declaration(ctx, each);
	DYNARR_FOREACH_END();
}

void cgc_external_declaration(struct cgc_context *ctx, struct external_declaration *external_decl) {
	if (external_decl->func_def_declarator != NULL) {
		cgc_function_definition(ctx, external_decl->decl_specifiers, external_decl->func_def_declarator, external_decl->compound_stmt);
	} else {
		cgc_declaration(ctx, external_decl->decl_specifiers, external_decl->init_declarator_list);
	}
}

void cgc_function_definition(struct cgc_context *ctx, struct declaration_specifiers* decl_specifiers, struct declarator *func_def_declarator, struct compound_statement *compound_stmt) {
	cgc_indent(ctx);
	cgc_declaration_specifiers(ctx, decl_specifiers);
	cgc_print(ctx, " ");
	cgc_declarator(ctx, func_def_declarator);
	cgc_print(ctx, "\n");
	cgc_compound_statement(ctx, compound_stmt);
}

void cgc_compound_statement(struct cgc_context *ctx, struct compound_statement *compound_stmt) {
	cgc_indent(ctx); cgc_print(ctx, "{\n");

	ctx->indent += ctx->step;

	DYNARR_FOREACH_BEGIN(compound_stmt->decl_or_stmt_list, syntreebasenode, each);
		if (each->nodeType == DECLARATION) {
			struct declaration *decl = (struct declaration *) each;
			cgc_declaration(ctx, decl->decl_specifiers, decl->init_declarator_list);
		} else {
			cgc_statement(ctx, each);
		}
	DYNARR_FOREACH_END();

	ctx->indent -= ctx->step;
	cgc_indent(ctx); cgc_print(ctx, "}\n");
}

void cgc_constant_value(struct cgc_context *ctx, union token tok) {
	int flags = tok.const_val.flags;
	if (flags & CONST_VAL_TOK_INTEGER) {
		cgc_print(ctx, "%d", tok.const_val.ival);
	} else if (flags & CONST_VAL_TOK_FLOAT) {
		cgc_print(ctx, "%f", tok.const_val.fval);
	} else if (flags & CONST_VAL_TOK_LONG_LONG) {
		cgc_print(ctx, "%lldll", tok.const_val.llval);
	} else {
		panic("invalid flag");
	}
}

void cgc_primary_expression(struct cgc_context *ctx, struct primary_expression *expr) {
	if (expr->id != NULL) {
		cgc_print(ctx, "%s", expr->id);
	} else if (expr->str) {
		cgc_print(ctx, "\"%s\"", expr->str);
	} else if (expr->expr) {
		cgc_print(ctx, "(");
		cgc_expression(ctx, expr->expr);
		cgc_print(ctx, ")");
	} else if (expr->const_val_tok.tok_tag != TOK_UNDEF) {
		cgc_constant_value(ctx, expr->const_val_tok);
	} else {
		panic("invalid primary expression");
	}
}

void cgc_argument_expression_list(struct cgc_context *ctx, struct argument_expression_list *arg_list) {
	struct dynarr *assign_expr_list = arg_list->list;
	int first = 1;
	DYNARR_FOREACH_BEGIN(assign_expr_list, assignment_expression, each);
		if (!first) {
			cgc_print(ctx, ", ");
		}
		first = 0;
		cgc_assignment_expression(ctx, each);
	DYNARR_FOREACH_END();
}

void cgc_postfix_expression_suffix(struct cgc_context *ctx, struct postfix_expression_suffix *suff) {
	if (suff->ind != NULL) {
		cgc_print(ctx, "[");
		cgc_expression(ctx, suff->ind);
		cgc_print(ctx, "]");
	} else if (suff->arg_list != NULL) {
		cgc_print(ctx, "(");
		cgc_argument_expression_list(ctx, suff->arg_list);
		cgc_print(ctx, ")");
	} else if (suff->dot_id) {
		cgc_print(ctx, ".%s", suff->dot_id);
	} else if (suff->ptr_id) {
		cgc_print(ctx, "->%s", suff->ptr_id);
	} else if (suff->is_inc) {
		cgc_print(ctx, "%s", cgc_get_op_str(TOK_INC));
	} else if (suff->is_dec) {
		cgc_print(ctx, "%s", cgc_get_op_str(TOK_DEC));
	} else {
		panic("invalid suffix");
	}
}

void cgc_postfix_expression(struct cgc_context *ctx, struct postfix_expression *expr) {
	cgc_primary_expression(ctx, expr->prim_expr);
	DYNARR_FOREACH_BEGIN(expr->suff_list, postfix_expression_suffix, each);
		cgc_postfix_expression_suffix(ctx, each);
	DYNARR_FOREACH_END();
}

void cgc_type_name(struct cgc_context *ctx, struct type_name *type_name) {
	cgc_specifier_qualifier_list(ctx, type_name->sqlist);
	if (type_name->declarator != NULL) {
		cgc_print(ctx, " ");
		cgc_declarator(ctx, type_name->declarator);
	}
}

void cgc_unary_expression(struct cgc_context *ctx, struct unary_expression *unary_expr) {
	assert(unary_expr->nodeType == UNARY_EXPRESSION);
	if (unary_expr->inc_unary != NULL) {
		cgc_print(ctx, "%s", cgc_get_op_str(TOK_INC));
		cgc_unary_expression(ctx, unary_expr->inc_unary);
	} else if (unary_expr->dec_unary != NULL) {
		cgc_print(ctx, "%s", cgc_get_op_str(TOK_DEC));
		cgc_unary_expression(ctx, unary_expr->dec_unary);
	} else if (unary_expr->unary_op != TOK_UNDEF) {
		cgc_print(ctx, "%s", cgc_get_op_str(unary_expr->unary_op));
		cgc_cast_expression(ctx, unary_expr->unary_op_cast);
	} else if (unary_expr->postfix_expr != NULL) {
		cgc_postfix_expression(ctx, unary_expr->postfix_expr);
	} else if (unary_expr->sizeof_expr != NULL) {
		cgc_print(ctx, "sizeof ");
		cgc_unary_expression(ctx, unary_expr->sizeof_expr);
	} else if (unary_expr->sizeof_type != NULL) {
		cgc_print(ctx, "sizeof(");
		cgc_type_name(ctx, unary_expr->sizeof_type);
		cgc_print(ctx, ")");
	} else {
		panic("invalid unary expression");
	}
}

void cgc_cast_expression(struct cgc_context *ctx, struct cast_expression *expr) {
	assert(expr->nodeType == CAST_EXPRESSION);
	if (expr->unary_expr != NULL) {
		cgc_unary_expression(ctx, expr->unary_expr);
	} else {
		cgc_print(ctx, "(");
		cgc_type_name(ctx, expr->type_name);
		cgc_print(ctx, ") ");
		cgc_cast_expression(ctx, expr->cast_expr);
	}
}

void cgc_multiplicative_expression(struct cgc_context *ctx, struct multiplicative_expression *expr) {
	assert(expr->nodeType == MULTIPLICATIVE_EXPRESSION);
	CGC_BINARY_OP_EXPR(ctx, expr, expr->cast_expr_list, cast_expression, expr->oplist, TOK_UNDEF);
}

void cgc_additive_expression(struct cgc_context *ctx, struct additive_expression *expr) {
	assert(expr->nodeType == ADDITIVE_EXPRESSION);
	CGC_BINARY_OP_EXPR(ctx, expr, expr->mul_expr_list, multiplicative_expression, expr->oplist, TOK_UNDEF);
}

void cgc_shift_expression(struct cgc_context *ctx, struct shift_expression *expr) {
	assert(expr->nodeType == SHIFT_EXPRESSION);
	CGC_BINARY_OP_EXPR(ctx, expr, expr->add_expr_list, additive_expression, expr->oplist, TOK_UNDEF);
}

void cgc_relational_expression(struct cgc_context *ctx, struct relational_expression *expr) {
	assert(expr->nodeType == RELATIONAL_EXPRESSION);
	CGC_BINARY_OP_EXPR(ctx, expr, expr->shift_expr_list, shift_expression, expr->oplist, TOK_UNDEF);
}

void cgc_equality_expression(struct cgc_context *ctx, struct equality_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->rel_expr_list, relational_expression, expr->oplist, TOK_UNDEF);
}

void cgc_and_expression(struct cgc_context *ctx, struct and_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->eq_expr_list, equality_expression, NULL, TOK_AMPERSAND);
}

void cgc_exclusive_or_expression(struct cgc_context *ctx, struct exclusive_or_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->and_expr_list, and_expression, NULL, TOK_XOR);
}

void cgc_inclusive_or_expression(struct cgc_context *ctx, struct inclusive_or_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->xor_expr_list, exclusive_or_expression, NULL, TOK_VERT_BAR);
}

void cgc_logical_and_expression(struct cgc_context *ctx, struct logical_and_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->or_expr_list, inclusive_or_expression, NULL, TOK_LOGIC_AND);
}

void cgc_logical_or_expression(struct cgc_context *ctx, struct logical_or_expression *expr) {
	CGC_BINARY_OP_EXPR(ctx, expr, expr->and_expr_list, logical_and_expression, NULL, TOK_LOGIC_OR);
}

void cgc_conditional_expression(struct cgc_context *ctx, struct conditional_expression *cond_expr) {
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

void cgc_assignment_expression(struct cgc_context *ctx, struct assignment_expression *expr) {
	int i;
	for (i = 0; i < dynarr_size(expr->unary_expr_list); i++) {
		struct unary_expression *unary_expr = dynarr_get(expr->unary_expr_list, i);
		int optok = (int)(long) dynarr_get(expr->oplist, i);
		cgc_unary_expression(ctx, unary_expr);
		cgc_print(ctx, " %s ", cgc_get_op_str(optok));
	}
	cgc_conditional_expression(ctx, expr->cond_expr);
}

void cgc_expression(struct cgc_context *ctx, struct expression *expr) {
	int first = 1;
	DYNARR_FOREACH_BEGIN(expr->darr, assignment_expression, each);
		if (!first) {
			cgc_print(ctx, ", ");
		}
		first = 0;
		cgc_assignment_expression(ctx, each);
	DYNARR_FOREACH_END();
}

void cgc_expression_statement(struct cgc_context *ctx, struct expression_statement *stmt, bool with_indent) {
	if (with_indent) {
		cgc_indent(ctx); 
	}
	if (stmt->expr != NULL)
		cgc_expression(ctx, stmt->expr);
	cgc_print(ctx, ";");
	if (with_indent) {
		cgc_print(ctx, "\n");
	}
}

void cgc_jump_statement(struct cgc_context *ctx, struct jump_statement *stmt) {
	cgc_indent(ctx);
	if (stmt->init_tok_tag == TOK_GOTO) {
		cgc_print(ctx, "goto %s", stmt->goto_label);
	} else if (stmt->init_tok_tag == TOK_CONTINUE) {
		cgc_print(ctx, "continue");
	} else if (stmt->init_tok_tag == TOK_BREAK) {
		cgc_print(ctx, "break");
	} else if (stmt->init_tok_tag == TOK_RETURN) {
		cgc_print(ctx, "return");
		if (stmt->ret_expr != NULL) {
			cgc_print(ctx, " ");
			cgc_expression(ctx, stmt->ret_expr);
		}
	}
	cgc_print(ctx, ";\n");
}

void cgc_indent_statement(struct cgc_context *ctx, struct statement *stmt) {
	if (stmt->nodeType != COMPOUND_STATEMENT) {
		ctx->indent += ctx->step;
		cgc_statement(ctx, stmt);
		ctx->indent -= ctx->step;
	} else {
		cgc_compound_statement(ctx, (struct compound_statement *) stmt);
	}
}

void cgc_while_statement(struct cgc_context *ctx, struct expression *expr, struct statement *stmt) {
	cgc_indent(ctx);
	cgc_print(ctx, "while (");
	cgc_expression(ctx, expr);
	cgc_print(ctx, ")\n");
	cgc_indent_statement(ctx, stmt);
}

void cgc_do_while_statement(struct cgc_context *ctx, struct statement *stmt, struct expression *expr) {
	cgc_indent(ctx);
	cgc_println(ctx, "do");
	cgc_indent_statement(ctx, stmt);
	cgc_indent(ctx);
	cgc_print(ctx, "while (");
	cgc_expression(ctx, expr);
	cgc_println(ctx, ");");
}

void cgc_for_statement(struct cgc_context *ctx, struct expression_statement *expr_stmt_1, struct expression_statement *expr_stmt_2, struct expression *expr, struct statement *stmt) {
	cgc_indent(ctx);
	cgc_print(ctx, "for (");
	cgc_expression_statement(ctx, expr_stmt_1, false);
	cgc_print(ctx, " ");
	cgc_expression_statement(ctx, expr_stmt_2, false);
	if (expr) {
		cgc_print(ctx, " ");
		cgc_expression(ctx, expr);
	}
	cgc_print(ctx, ")\n");
	cgc_indent_statement(ctx, stmt);
}

void cgc_iteration_statement(struct cgc_context *ctx, struct iteration_statement *stmt) {
	switch (stmt->iterType) {
	case ITER_TYPE_WHILE:
		cgc_while_statement(ctx, stmt->while_stmt.expr, stmt->while_stmt.stmt);
		break;
	case ITER_TYPE_DO_WHILE:
		cgc_do_while_statement(ctx, stmt->do_while_stmt.stmt, stmt->do_while_stmt.expr);
		break;
	case ITER_TYPE_FOR:
		cgc_for_statement(ctx, stmt->for_stmt.expr_stmt_1, stmt->for_stmt.expr_stmt_2, stmt->for_stmt.expr, stmt->for_stmt.stmt);
		break;
	default:
		panic("invalid iter type %d", stmt->iterType);
	}
}

void cgc_if_statement(struct cgc_context *ctx, struct expression *expr, struct statement *truestmt, struct statement *falsestmt) {
	cgc_indent(ctx);
	cgc_print(ctx, "if (");
	cgc_expression(ctx, expr);
	cgc_print(ctx, ")\n");
	assert(truestmt != NULL);
	cgc_indent_statement(ctx, truestmt);
	if (falsestmt) {
		cgc_indent(ctx);
		cgc_print(ctx, "else\n");
		cgc_indent_statement(ctx, falsestmt);
	}
}

void cgc_switch_statement(struct cgc_context *ctx, struct expression *expr, struct statement *stmt) {
	cgc_indent(ctx);
	cgc_print(ctx, "switch (");
	cgc_expression(ctx, expr);
	cgc_print(ctx, ")\n");
	cgc_indent_statement(ctx, stmt);
}

void cgc_selection_statement(struct cgc_context *ctx, struct selection_statement *stmt) {
	switch (stmt->selType) {
	case SEL_TYPE_IF:
		cgc_if_statement(ctx, stmt->if_stmt.expr, stmt->if_stmt.truestmt, stmt->if_stmt.falsestmt);
		break;
	case SEL_TYPE_SWITCH:
		cgc_switch_statement(ctx, stmt->switch_stmt.expr, stmt->switch_stmt.stmt);
		break;
	default:
		panic("invalid sel type");
	}
}

void cgc_labeled_statement(struct cgc_context *ctx, struct labeled_statement *labeled_stmt) {
	assert(labeled_stmt->nodeType == LABELED_STATEMENT);
	switch (labeled_stmt->init_tok) {
	case TOK_IDENTIFIER:
		cgc_println(ctx, "%s:", labeled_stmt->label_str); // no indent
		cgc_statement(ctx, labeled_stmt->stmt);
		break;
	case TOK_CASE:
		cgc_indent(ctx);
		cgc_print(ctx, "case ");
		cgc_constant_expression(ctx, labeled_stmt->case_expr);
		cgc_println(ctx, ":");
		cgc_indent_statement(ctx, labeled_stmt->stmt);
		break;
	case TOK_DEFAULT:
		cgc_indent(ctx);
		cgc_println(ctx, "default:");
		cgc_indent_statement(ctx, labeled_stmt->stmt);
		break;
	default:
		panic("can not reach here, %s", token_tag_str(labeled_stmt->nodeType));
	}
}

void cgc_statement(struct cgc_context *ctx, struct syntreebasenode *stmt) {
	switch (stmt->nodeType) {
	case EXPRESSION_STATEMENT:
		cgc_expression_statement(ctx, (struct expression_statement *) stmt, true);
		break;
	case JUMP_STATEMENT:
		cgc_jump_statement(ctx, (struct jump_statement *) stmt);
		break;
	case ITERATION_STATEMENT:
		cgc_iteration_statement(ctx, (struct iteration_statement *) stmt);
		break;
	case SELECTION_STATEMENT:
		cgc_selection_statement(ctx, (struct selection_statement *) stmt);
		break;
	case LABELED_STATEMENT:
		cgc_labeled_statement(ctx, (struct labeled_statement *) stmt);
		break;
	case COMPOUND_STATEMENT:
		cgc_compound_statement(ctx, (struct compound_statement *) stmt);
		break;
	default:
		panic("unexpected node type %s", node_type_str(stmt->nodeType));
	}
}

void cgc_pointer(struct cgc_context *ctx, struct dynarr *ptr_list) {
	DYNARR_FOREACH_BEGIN(ptr_list, type_qualifier_list, each);
		cgc_print(ctx, "*");
		int j;
		for (j = 0; j < dynarr_size(each->darr); j++) {
			struct type_qualifier *qual = dynarr_get(each->darr, j);
			int qual_tok = qual->tok_tag;
			cgc_print(ctx, "%s ", keyword_str(qual_tok));
		}
	DYNARR_FOREACH_END();
}

void cgc_declarator(struct cgc_context *ctx, struct declarator *declarator) {
	cgc_pointer(ctx, declarator->ptr_list);
	cgc_direct_declarator(ctx, declarator->direct_declarator);
}

void cgc_parameter_declaration(struct cgc_context *ctx, struct parameter_declaration *decl) {
	cgc_declaration_specifiers(ctx, decl->decl_specifiers);	
	if (decl->declarator) {
		cgc_print(ctx, " ");
		cgc_declarator(ctx, decl->declarator);
	}
}

void cgc_parameter_type_list(struct cgc_context *ctx, struct parameter_type_list *list) {
	int first = 1;
	DYNARR_FOREACH_BEGIN(list->param_decl_list, parameter_declaration, each);
		if (!first) {
			cgc_print(ctx, ", ");
		}
		first = 0;
		cgc_parameter_declaration(ctx, each);
	DYNARR_FOREACH_END();
	if (list->has_ellipsis) {
		cgc_print(ctx, ", ...");
	}
}

void cgc_direct_declarator(struct cgc_context *ctx, struct direct_declarator *direct_declarator) {
	if (direct_declarator->id != NULL) {
		cgc_print(ctx, "%s", direct_declarator->id);
	} else if (direct_declarator->declarator != NULL) {
		cgc_print(ctx, "(");
		cgc_declarator(ctx, direct_declarator->declarator);
		cgc_print(ctx, ")");
	} else {
		// fall thru
	}

	DYNARR_FOREACH_BEGIN(direct_declarator->suff_list, direct_declarator_suffix, suff);
		if (suff->empty_bracket) {
			cgc_print(ctx, "[]");
		} else if (suff->empty_paren) {
			cgc_print(ctx, "()");
		} else if (suff->const_expr) {
			cgc_print(ctx, "[");
			cgc_constant_expression(ctx, suff->const_expr);
			cgc_print(ctx, "]");
		} else if (suff->param_type_list) {
			cgc_print(ctx, "(");
			cgc_parameter_type_list(ctx, suff->param_type_list);
			cgc_print(ctx, ")");
		} else {
			panic("invalid declaraotr suffix");
		}
	DYNARR_FOREACH_END();
}

void cgc_declaration(struct cgc_context *ctx, struct declaration_specifiers *decl_specifiers, struct init_declarator_list *init_declarator_list) {
	cgc_indent(ctx);
	cgc_declaration_specifiers(ctx, decl_specifiers);

	if (init_declarator_list != NULL) {
		cgc_print(ctx, " ");
		cgc_init_declarator_list(ctx, init_declarator_list);
	}
	cgc_print(ctx, ";\n");
}

void cgc_init_declarator_list(struct cgc_context *ctx, struct init_declarator_list *init_declarator_list) {
	int is_first = 1;
	DYNARR_FOREACH_BEGIN(init_declarator_list->darr, init_declarator, each);
		if (!is_first) {
			cgc_print(ctx, ", ");
		}
		is_first = 0;
		cgc_init_declarator(ctx, each);
	DYNARR_FOREACH_END();
}

void cgc_initializer_list(struct cgc_context *ctx, struct initializer_list *initz_list) {
	cgc_println(ctx, "{");
	ctx->indent += ctx->step;

	DYNARR_FOREACH_BEGIN(initz_list->list, initializer, each);
		cgc_indent(ctx);
		cgc_initializer(ctx, each);
		cgc_println(ctx, ",");
	DYNARR_FOREACH_END();

	ctx->indent -= ctx->step;
	cgc_indent(ctx); cgc_print(ctx, "}");
}

void cgc_initializer(struct cgc_context *ctx, struct initializer *initializer) {
	if (initializer->expr != NULL) {
		cgc_assignment_expression(ctx, initializer->expr);
	} else {
		cgc_initializer_list(ctx, initializer->initz_list);
	}
}

void cgc_init_declarator(struct cgc_context *ctx, struct init_declarator *init_declarator) {
	cgc_declarator(ctx, init_declarator->declarator);

	if (init_declarator->initializer != NULL) {
		cgc_print(ctx, " = ");
		cgc_initializer(ctx, init_declarator->initializer);
	}
}

void cgc_type_specifier_qualifier_sc(struct cgc_context *ctx, struct dynarr *darr) {
	int first = 1;
	DYNARR_FOREACH_BEGIN(darr, syntreebasenode, each);
		if (!first) {
			cgc_print(ctx, " ");
		}
		first = 0;
		switch (each->nodeType) {
		case TYPE_SPECIFIER:
			cgc_type_specifier(ctx, (struct type_specifier *) each);
			break;
		case TYPE_QUALIFIER:
			cgc_type_qualifier(ctx, (struct type_qualifier *) each);
			break;
		case STORAGE_CLASS_SPECIFIER:
			cgc_storage_class_specifier(ctx, (struct storage_class_specifier *) each);
			break;
		default:
			panic("ni %s", node_type_str(each->nodeType));
		}
	DYNARR_FOREACH_END();
}

void cgc_declaration_specifiers(struct cgc_context *ctx, struct declaration_specifiers *decl_specifiers) {
	cgc_type_specifier_qualifier_sc(ctx, decl_specifiers->darr);
}

void cgc_specifier_qualifier_list(struct cgc_context *ctx, struct specifier_qualifier_list *list) {
	cgc_type_specifier_qualifier_sc(ctx, list->darr);
}

void cgc_struct_declarator_list(struct cgc_context *ctx, struct dynarr *struct_declarator_list) {
	int is_first = 1;
	DYNARR_FOREACH_BEGIN(struct_declarator_list, struct_declarator, each);
		if (!is_first) {
			cgc_print(ctx, ", ");
		}
		is_first = 0;

		if (each->declarator != NULL) {
			cgc_declarator(ctx, each->declarator);
		}
		if (each->const_expr != NULL) {
			cgc_print(ctx, " : ");
			cgc_constant_expression(ctx, each->const_expr);
		}
	DYNARR_FOREACH_END();
}

void cgc_struct_declaration(struct cgc_context *ctx, struct struct_declaration *field) {
	cgc_indent(ctx);
	cgc_specifier_qualifier_list(ctx, field->sqlist);
	cgc_print(ctx, " ");
	cgc_struct_declarator_list(ctx, field->declarator_list);
	cgc_print(ctx, ";\n");
}

/* support both struct and union */
void cgc_struct(struct cgc_context *ctx, struct type_specifier *type_specifier) {
	cgc_print(ctx, type_specifier->tok_tag == TOK_STRUCT ? "struct" : "union");
	if (type_specifier->type_name != NULL) {
		cgc_print(ctx, " %s", type_specifier->type_name);
	} 
	if (type_specifier->struct_decl_list != NULL) {
		cgc_print(ctx, " {\n");
		ctx->indent += ctx->step;
		DYNARR_FOREACH_BEGIN(type_specifier->struct_decl_list->decl_list, struct_declaration, each);
			cgc_struct_declaration(ctx, each);
		DYNARR_FOREACH_END();
		ctx->indent -= ctx->step;
		cgc_indent(ctx), cgc_print(ctx, "}");
	}
}

void cgc_enumerator(struct cgc_context *ctx, struct enumerator *enumerator) {
	cgc_indent(ctx);
	cgc_print(ctx, "%s", enumerator->name);
	if (enumerator->expr) {
		cgc_print(ctx, " = ");
		cgc_constant_expression(ctx, enumerator->expr);
	}
	cgc_print(ctx, ",\n");
}

void cgc_enum(struct cgc_context *ctx, struct type_specifier *enum_specifier) {
	cgc_print(ctx, "enum");
	if (enum_specifier->type_name != NULL) {
		cgc_print(ctx, " %s", enum_specifier->type_name);
	}
	if (enum_specifier->enumerator_list != NULL) {
		cgc_print(ctx, " {\n");
		ctx->indent += ctx->step;
		DYNARR_FOREACH_BEGIN(enum_specifier->enumerator_list->enum_list, enumerator, each);
			cgc_enumerator(ctx, each);
		DYNARR_FOREACH_END();
		ctx->indent -= ctx->step;
		cgc_indent(ctx), cgc_print(ctx, "}");
	}
}

void cgc_type_specifier(struct cgc_context *ctx, struct type_specifier *type_specifier) {
	switch (type_specifier->tok_tag) {
	case TOK_INT: cgc_print(ctx, "int"); break;
	case TOK_CHAR: cgc_print(ctx, "char"); break;
	case TOK_LONG: cgc_print(ctx, "long"); break;
	case TOK_UNSIGNED: cgc_print(ctx, "unsigned"); break;
	case TOK_SIGNED: cgc_print(ctx, "signed"); break;
	case TOK_SHORT: cgc_print(ctx, "short"); break;
	case TOK_VOID: cgc_print(ctx, "void"); break;
	case TOK_DOUBLE: cgc_print(ctx, "double"); break;
	case TOK_FLOAT: cgc_print(ctx, "float"); break;
	case TOK_TYPE_NAME: 
		cgc_print(ctx, "%s", type_specifier->type_name); 
		break;
	case TOK_STRUCT:
	case TOK_UNION:
		cgc_struct(ctx, type_specifier);
		break;
	case TOK_ENUM:
		cgc_enum(ctx, type_specifier);
		break;
	default:
		panic("ni %s", token_tag_str(type_specifier->tok_tag));
	}
}

void cgc_storage_class_specifier(struct cgc_context *ctx, struct storage_class_specifier *sc) {
	switch (sc->tok_tag) {
	case TOK_TYPEDEF:
		cgc_print(ctx, "typedef");
		break;
	case TOK_EXTERN:
		cgc_print(ctx, "extern");
		break;
	case TOK_STATIC:
		cgc_print(ctx, "static");
		break;
	case TOK_REGISTER:
		cgc_print(ctx, "register");
		break;
	case TOK_INLINE:
		cgc_print(ctx, "inline");
		break;
	default:
		panic("ni %s", token_tag_str(sc->tok_tag));
	}
}

void cgc_type_qualifier(struct cgc_context *ctx, struct type_qualifier *qual) {
	switch (qual->tok_tag) {
	case TOK_CONST:
		cgc_print(ctx, "const");
		break;
	case TOK_VOLATILE:
		cgc_print(ctx, "volatile");
		break;
	default:
		panic("ni");
	}
}

