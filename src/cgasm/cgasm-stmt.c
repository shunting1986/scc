#include <inc/cgasm.h>
#include <inc/syntree.h>
#include <inc/util.h>


static void cgasm_statement(struct cgasm_context *ctx, struct syntreebasenode *stmt);

static void cgasm_return_statement(struct cgasm_context *ctx, struct expression *expr) {
	if (expr != NULL) {
		struct expr_val val = cgasm_expression(ctx, expr);
		cgasm_load_val_to_reg(ctx, val, REG_EAX);
	}
	cgasm_handle_ret(ctx);
}

static void cgasm_break_statement(struct cgasm_context *ctx) {
	if (intstack_size(ctx->break_label_stk) == 0) {
		panic("invalid 'break'");
	}

	int label = intstack_top(ctx->break_label_stk);
	char buf[128];
	cgasm_println(ctx, "jmp %s", get_jump_label_str(label, buf));
}

static void cgasm_continue_statement(struct cgasm_context *ctx) {
	if (intstack_size(ctx->continue_label_stk) == 0) {
		panic("invalid 'continue'");
	}

	int label = intstack_top(ctx->continue_label_stk);
	char buf[128];
	cgasm_println(ctx, "jmp %s", get_jump_label_str(label, buf));
}

static void cgasm_jump_statement(struct cgasm_context *ctx, struct jump_statement *stmt) {
	switch (stmt->init_tok_tag) {
	case TOK_RETURN:
		cgasm_return_statement(ctx, stmt->ret_expr);
		break;
	case TOK_BREAK:
		cgasm_break_statement(ctx);
		break;
	case TOK_CONTINUE:
		cgasm_continue_statement(ctx);
		break;
	default:
		panic("ni %s", token_tag_str(stmt->init_tok_tag));
	}
}

/* 
 * when expression_statement is used in for statement, we will need the expression part
 */
static struct expr_val cgasm_expression_statement(struct cgasm_context *ctx, struct expression_statement *stmt) {
	if (stmt->expr == NULL) {
		return void_expr_val();
	}
	return cgasm_expression(ctx, stmt->expr);
}

static void cgasm_goto_ifcond_logic_and(struct cgasm_context *ctx, struct expr_val lhs, struct syntreebasenode *rhs_lazy, int goto_label, int itemreverse) {
	int out_label = cgasm_new_label_no(ctx);
	char buf[128];

	cgasm_goto_ifcond(ctx, lhs, out_label, 1 - itemreverse);
	cgasm_goto_ifcond(ctx, cgasm_eval_expr(ctx, rhs_lazy), out_label, 1 - itemreverse);
	cgasm_println(ctx, "jmp %s", get_jump_label_str(goto_label, buf));
	cgasm_emit_jump_label(ctx, out_label);
}

static void cgasm_goto_ifcond_logic_or(struct cgasm_context *ctx, struct expr_val lhs, struct syntreebasenode *rhs_lazy, int goto_label, int itemreverse) {
	cgasm_goto_ifcond(ctx, lhs, goto_label, itemreverse);
	cgasm_goto_ifcond(ctx, cgasm_eval_expr(ctx, rhs_lazy), goto_label, itemreverse);
}

static void cgasm_goto_ifcond_logic(struct cgasm_context *ctx, int op, struct expr_val lhs, struct syntreebasenode *rhs_lazy, int goto_label, int reverse) {
	assert(op == TOK_LOGIC_AND || op == TOK_LOGIC_OR);
	int finalop = reverse ? TOK_LOGIC_AND + TOK_LOGIC_OR - op : op;
	int itemreverse = reverse;

	if (finalop == TOK_LOGIC_AND) {
		cgasm_goto_ifcond_logic_and(ctx, lhs, rhs_lazy, goto_label, itemreverse);
	} else {
		cgasm_goto_ifcond_logic_or(ctx, lhs, rhs_lazy, goto_label, itemreverse);
	}
}

void cgasm_goto_ifcond_cc(struct cgasm_context *ctx, struct condcode *ccexpr, int goto_label, int reverse) {
	int op = ccexpr->op;
	int lhs_reg = REG_EAX;
	int rhs_reg = REG_ECX;
	char buf[128];
	struct expr_val lhs = ccexpr->lhs;
	struct expr_val rhs = ccexpr->rhs;
	struct syntreebasenode *rhs_lazy = ccexpr->rhs_lazy;
	free(ccexpr); // free condcode after evaluation
	ccexpr = NULL;

	if (op == TOK_EXCLAMATION) {
		cgasm_goto_ifcond(ctx, lhs, goto_label, 1 - reverse);
		return;
	}

	if (op == TOK_LOGIC_AND || op == TOK_LOGIC_OR) {
		cgasm_goto_ifcond_logic(ctx, op, lhs, rhs_lazy, goto_label, reverse);
		return;
	}

	// handle reverse
	if (reverse) {
		switch (op) {
		case TOK_NE: op = TOK_EQ; break;
		case TOK_LE: op = TOK_GT; break;
		case TOK_GT: op = TOK_LE; break;
		case TOK_LT: op = TOK_GE; break;
		case TOK_GE: op = TOK_LT; break;
		case TOK_EQ: op = TOK_NE; break;
		default:
			panic("ni %s", token_tag_str(op));
		}
	}

	cgasm_load_val_to_reg(ctx, lhs, lhs_reg);
	cgasm_load_val_to_reg(ctx, rhs, rhs_reg);
	switch (op) {
	case TOK_EQ:
		cgasm_println(ctx, "cmpl %%%s, %%%s", get_reg_str_code(rhs_reg), get_reg_str_code(lhs_reg));
		cgasm_println(ctx, "jz %s", get_jump_label_str(goto_label, buf));
		break;
	case TOK_NE:
		cgasm_println(ctx, "cmpl %%%s, %%%s", get_reg_str_code(rhs_reg), get_reg_str_code(lhs_reg));
		cgasm_println(ctx, "jnz %s", get_jump_label_str(goto_label, buf));
		break;
	case TOK_GT:
		cgasm_println(ctx, "cmpl %%%s, %%%s", get_reg_str_code(rhs_reg), get_reg_str_code(lhs_reg));
		cgasm_println(ctx, "jg %s", get_jump_label_str(goto_label, buf));
		break;
	case TOK_LE:
		cgasm_println(ctx, "cmpl %%%s, %%%s", get_reg_str_code(rhs_reg), get_reg_str_code(lhs_reg));
		cgasm_println(ctx, "jle %s", get_jump_label_str(goto_label, buf));
		break;
	case TOK_GE:
		cgasm_println(ctx, "cmpl %%%s, %%%s", get_reg_str_code(rhs_reg), get_reg_str_code(lhs_reg));
		cgasm_println(ctx, "jge %s", get_jump_label_str(goto_label, buf));
		break;
	case TOK_LT:
		cgasm_println(ctx, "cmpl %%%s, %%%s", get_reg_str_code(rhs_reg), get_reg_str_code(lhs_reg));
		cgasm_println(ctx, "jl %s", get_jump_label_str(goto_label, buf));
		break;
	default:
		panic("ni %s", token_tag_str(op));
	}
}


void cgasm_goto_ifcond(struct cgasm_context *ctx, struct expr_val condval, int goto_label, int inverse) {
	char buf[128];
	(void) buf;

	switch (condval.type) {
	case EXPR_VAL_CC:
		cgasm_goto_ifcond_cc(ctx, condval.cc, goto_label, inverse);
		break;
	case EXPR_VAL_SYMBOL: case EXPR_VAL_TEMP:
		cgasm_goto_ifcond_cc(ctx, 
			condcode_expr(TOK_NE, condval, int_const_expr_val(0), NULL).cc,
			goto_label, inverse);
		break;
	default:
		panic("ni %d", condval.type);
	}
}

static void cgasm_while_statement(struct cgasm_context *ctx, struct expression *expr, struct statement *stmt) {
	int entry_label = cgasm_new_label_no(ctx);
	int exit_label = cgasm_new_label_no(ctx);
	char buf[128];
	struct expr_val cond_expr_val;

	cgasm_emit_jump_label(ctx, entry_label);
	cond_expr_val = cgasm_expression(ctx, expr);
	cgasm_goto_ifcond(ctx, cond_expr_val, exit_label, true);

	cgasm_push_break_label(ctx, exit_label);
	cgasm_push_continue_label(ctx, entry_label);
	cgasm_statement(ctx, stmt);
	assert(cgasm_pop_break_label(ctx) == exit_label);
	assert(cgasm_pop_continue_label(ctx) == entry_label);

	cgasm_println(ctx, "jmp %s", get_jump_label_str(entry_label, buf));
	cgasm_emit_jump_label(ctx, exit_label);
}

static void cgasm_do_while_statement(struct cgasm_context *ctx, struct statement *stmt, struct expression *expr) {
	struct expr_val cond_expr_val;
	int entry_label = cgasm_new_label_no(ctx);
	int exit_label = cgasm_new_label_no(ctx);
	int continue_label = cgasm_new_label_no(ctx);

	cgasm_emit_jump_label(ctx, entry_label);

	cgasm_push_break_label(ctx, exit_label);
	cgasm_push_continue_label(ctx, continue_label);
	cgasm_statement(ctx, stmt); // this is the body
	assert(cgasm_pop_break_label(ctx) == exit_label);
	assert(cgasm_pop_continue_label(ctx) == continue_label);

	cgasm_emit_jump_label(ctx, continue_label);
	cond_expr_val = cgasm_expression(ctx, expr);
	cgasm_goto_ifcond(ctx, cond_expr_val, entry_label, false);

	cgasm_emit_jump_label(ctx, exit_label);
}

static void cgasm_for_statement(struct cgasm_context *ctx, struct expression_statement *expr_stmt1, struct expression_statement *expr_stmt2, struct expression *expr, struct statement *stmt) {
	int entry_label = cgasm_new_label_no(ctx);
	int exit_label = cgasm_new_label_no(ctx);
	int continue_label = cgasm_new_label_no(ctx);
	struct expr_val cond_expr_val;
	char buf[128];
	
	cgasm_expression_statement(ctx, expr_stmt1);
	cgasm_emit_jump_label(ctx, entry_label);
	cond_expr_val = cgasm_expression_statement(ctx, expr_stmt2);
	cgasm_goto_ifcond(ctx, cond_expr_val, exit_label, true);

	cgasm_push_break_label(ctx, exit_label);
	cgasm_push_continue_label(ctx, continue_label);
	cgasm_statement(ctx, stmt);
	
	assert(cgasm_pop_break_label(ctx) == exit_label);
	assert(cgasm_pop_continue_label(ctx) == continue_label);

	cgasm_emit_jump_label(ctx, continue_label);
	if (expr) {
		cgasm_expression(ctx, expr);
	}
	cgasm_println(ctx, "jmp %s", get_jump_label_str(entry_label, buf));
	cgasm_emit_jump_label(ctx, exit_label);
}

static void cgasm_iteration_statement(struct cgasm_context *ctx, struct iteration_statement *stmt) {
	switch (stmt->iterType) {
	case ITER_TYPE_WHILE:
		cgasm_while_statement(ctx, stmt->while_stmt.expr, stmt->while_stmt.stmt);
		break;
	case ITER_TYPE_FOR:
		cgasm_for_statement(ctx, stmt->for_stmt.expr_stmt_1, stmt->for_stmt.expr_stmt_2, stmt->for_stmt.expr, stmt->for_stmt.stmt);
		break;
	case ITER_TYPE_DO_WHILE:
		cgasm_do_while_statement(ctx, stmt->do_while_stmt.stmt, stmt->do_while_stmt.expr);
		break;
	default:
		panic("ni %d", stmt->iterType);
	}
}

/*
 * handle the if statement without else
 */
static void cgasm_if_statement(struct cgasm_context *ctx, struct expression *cond, struct statement *stmt) {
	int exit_label = cgasm_new_label_no(ctx);
	struct expr_val cond_expr_val;

	cond_expr_val = cgasm_expression(ctx, cond);
	cgasm_goto_ifcond(ctx, cond_expr_val, exit_label, true);
	cgasm_statement(ctx, stmt);
	cgasm_emit_jump_label(ctx, exit_label);
}

/*
 * handle the if statement with else
 */
static void cgasm_ifelse_statement(struct cgasm_context *ctx, struct expression *cond, struct statement *truestmt, struct statement *falsestmt) {
	int else_label = cgasm_new_label_no(ctx);
	int exit_label = cgasm_new_label_no(ctx);
	char buf[128];
	struct expr_val cond_expr_val;

	cond_expr_val = cgasm_expression(ctx, cond);
	cgasm_goto_ifcond(ctx, cond_expr_val, else_label, true);
	cgasm_statement(ctx, truestmt);
	cgasm_println(ctx, "jmp %s", get_jump_label_str(exit_label, buf));
	cgasm_emit_jump_label(ctx, else_label);
	cgasm_statement(ctx, falsestmt);
	cgasm_emit_jump_label(ctx, exit_label);
}

static void cgasm_switch_statement(struct cgasm_context *ctx, struct expression *expr, struct statement *stmt) {
	panic("ni");
}

static void cgasm_selection_statement(struct cgasm_context *ctx, struct selection_statement *stmt) {
	switch (stmt->selType) {
	case SEL_TYPE_IF:
		if (stmt->if_stmt.falsestmt != NULL) {
			cgasm_ifelse_statement(ctx, stmt->if_stmt.expr, stmt->if_stmt.truestmt, stmt->if_stmt.falsestmt);
		} else {
			cgasm_if_statement(ctx, stmt->if_stmt.expr, stmt->if_stmt.truestmt);
		}
		break;
	case SEL_TYPE_SWITCH:
		cgasm_switch_statement(ctx, stmt->switch_stmt.expr, stmt->switch_stmt.stmt);
		break;
	default:
		panic("ni");
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
	case SELECTION_STATEMENT:
		cgasm_selection_statement(ctx, (struct selection_statement *) stmt);
		break;
	case COMPOUND_STATEMENT:
		cgasm_push_symtab(ctx);
		cgasm_compound_statement(ctx, (struct compound_statement *) stmt);
		cgasm_pop_symtab(ctx);
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


