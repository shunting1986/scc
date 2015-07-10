#include <inc/cgasm.h>
#include <inc/syntree.h>
#include <inc/util.h>

static void cgasm_return_statement(struct cgasm_context *ctx, struct expression *expr) {
	if (expr != NULL) {
		struct expr_val val = cgasm_expression(ctx, expr);
		cgasm_load_val_to_reg(ctx, val, REG_EAX);
	}
	cgasm_handle_ret(ctx);
}

static void cgasm_jump_statement(struct cgasm_context *ctx, struct jump_statement *stmt) {
	switch (stmt->init_tok_tag) {
	case TOK_RETURN:
		cgasm_return_statement(ctx, stmt->ret_expr);
		break;
	default:
		panic("ni %s", token_tag_str(stmt->init_tok_tag));
	}
}

static void cgasm_expression_statement(struct cgasm_context *ctx, struct expression_statement *stmt) {
	if (stmt->expr == NULL) {
		return;
	}
	(void) cgasm_expression(ctx, stmt->expr);
}

static void cgasm_while_statement(struct cgasm_context *ctx, struct expression *expr, struct statement *stmt) {
	panic("ni");
}

static void cgasm_iteration_statement(struct cgasm_context *ctx, struct iteration_statement *stmt) {
	switch (stmt->iterType) {
	case ITER_TYPE_WHILE:
		cgasm_while_statement(ctx, stmt->while_stmt.expr, stmt->while_stmt.stmt);
		break;
	default:
		panic("ni %d", stmt->iterType);
	}
}

static void cgasm_statement(struct cgasm_context *ctx, struct syntreebasenode *stmt) {
	switch (stmt->nodeType) {
	case EXPRESSION_STATEMENT:
		cgasm_expression_statement(ctx, (struct expression_statement *) stmt);
		break;
	case JUMP_STATEMENT:
		cgasm_jump_statement(ctx, (struct jump_statement *) stmt);
		break;
	case ITERATION_STATEMENT:
		cgasm_iteration_statement(ctx, (struct iteration_statement *) stmt);
		break;
	default:
		panic("unexpected node type %s\n", node_type_str(stmt->nodeType));
	}
}

/* 
 * NOTE: the caller should create the symtab before calling this method and destroy it
 * afterwards
 */
void cgasm_compound_statement(struct cgasm_context *ctx, struct compound_statement *compound_stmt) {
	DYNARR_FOREACH_BEGIN(compound_stmt->declList, declaration, each);
		cgasm_declaration(ctx, each->decl_specifiers, each->init_declarator_list);
	DYNARR_FOREACH_END();
	
	DYNARR_FOREACH_BEGIN(compound_stmt->stmtList, syntreebasenode, each);
		cgasm_statement(ctx, each);
	DYNARR_FOREACH_END();
}


