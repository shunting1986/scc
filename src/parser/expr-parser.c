/*
 * Handle the parsing for expression
 */
#include <inc/parser.h>
#include <inc/lexer.h>
#include <inc/util.h>
#include <inc/dynarr.h>

static struct assignment_expression *parse_assignment_expression(struct parser *parser);

static struct primary_expression *parse_primary_expression(struct parser *parser) {
	union token tok = lexer_next_token(parser->lexer);
	struct primary_expression *prim_expr = primary_expression_init();
	if (tok.tok_tag == TOK_IDENTIFIER) {
		prim_expr->id = tok.id.s;
		return prim_expr;
	} else if (tok.tok_tag == TOK_STRING_LITERAL) {
		prim_expr->str = tok.str.s;
		return prim_expr;
	}
	panic("parse_primary_expression ni");
}

static struct argument_expression_list *parse_argument_expression_list(struct parser *parser) {
	struct argument_expression_list *arg_list = argument_expression_list_init();
	union token tok = lexer_next_token(parser->lexer);
	if (tok.tok_tag == TOK_RPAREN) {
		return arg_list;
	}
	lexer_put_back(parser->lexer, tok);
	while (1) {
		struct assignment_expression *assign_expr = parse_assignment_expression(parser);
		dynarr_add(arg_list->list, assign_expr);
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag == TOK_RPAREN) {
			break;
		} else if (tok.tok_tag != TOK_COMMA) {
			panic("argument expression list expects ',', but %s found", token_tag_str(tok.tok_tag));
		}
	}
	return arg_list;
}

static struct postfix_expression *parse_postfix_expression(struct parser *parser) {
	struct primary_expression *prim_expr = parse_primary_expression(parser);
	struct postfix_expression *post_expr = postfix_expression_init(prim_expr);
	union token tok = lexer_next_token(parser->lexer);
	while (1) {
		if (tok.tok_tag == TOK_LPAREN) {
			struct argument_expression_list *arg_expr_list = parse_argument_expression_list(parser);
			struct postfix_expression_suffix *suf = mallocz(sizeof(*suf));
			suf->arg_list = arg_expr_list;
			dynarr_add(post_expr->suff_list, suf);
		} else {
			lexer_put_back(parser->lexer, tok);
			break;
		}
	}
	return post_expr;
}

static struct unary_expression *parse_unary_expression(struct parser *parser) {
	// TODO handle other alternatives
	struct postfix_expression *post_expr = parse_postfix_expression(parser);

	struct unary_expression *unary_expr = unary_expression_init(parser);
	unary_expr->postfix_expr = post_expr;
	return unary_expr;
}

// only handle the case for unary_expression right now
static struct cast_expression *parse_cast_expression(struct parser *parser) {
	union token tok = lexer_next_token(parser->lexer);
	if (tok.tok_tag == TOK_LPAREN) {
		panic("parse_cast_expression does not support starting with '(' yet"); // TODO
	}
	lexer_put_back(parser->lexer, tok);

	struct cast_expression *cast_expr = cast_expression_init();
	cast_expr->unary_expr = parse_unary_expression(parser);
	return cast_expr;
}

static struct multiplicative_expression *parse_multiplicative_expression(struct parser *parser) {
	struct cast_expression *cast_expr = parse_cast_expression(parser);
	struct multiplicative_expression *mul_expr = multiplicative_expression_init(cast_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_STAR && tok.tok_tag != TOK_DIV && tok.tok_tag != TOK_MOD) {
			lexer_put_back(parser->lexer, tok);
			break; 
		}

		dynarr_add(mul_expr->oplist, (void *) (long) tok.tok_tag);
		dynarr_add(mul_expr->cast_expr_list, parse_cast_expression(parser));
	}
	return mul_expr;
}

static struct additive_expression *parse_additive_expression(struct parser *parser) {
	struct multiplicative_expression *mul_expr = parse_multiplicative_expression(parser);
	struct additive_expression *add_expr = additive_expression_init(mul_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_ADD && tok.tok_tag != TOK_SUB) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		dynarr_add(add_expr->oplist, (void *) (long) tok.tok_tag);
		dynarr_add(add_expr->mul_expr_list, parse_multiplicative_expression(parser));
	}
	return add_expr;
}

static struct shift_expression *parse_shift_expression(struct parser *parser) {
	struct additive_expression *add_expr = parse_additive_expression(parser);
	struct shift_expression *shift_expr = shift_expression_init(add_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_LSHIFT && tok.tok_tag != TOK_RSHIFT) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		dynarr_add(shift_expr->oplist, (void *) (long) tok.tok_tag);
		dynarr_add(shift_expr->add_expr_list, parse_shift_expression(parser));
	}
	return shift_expr;
}

static struct relational_expression *parse_relational_expression(struct parser *parser) {
	struct shift_expression *shift_expr = parse_shift_expression(parser);
	struct relational_expression *rel_expr = relational_expression_init(shift_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_LT && tok.tok_tag != TOK_LE && tok.tok_tag != TOK_GT && tok.tok_tag != TOK_GE) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		dynarr_add(rel_expr->oplist, (void *) (long) tok.tok_tag);
		dynarr_add(rel_expr->shift_expr_list, parse_shift_expression(parser));
	}
	return rel_expr;
}

static struct equality_expression *parse_equality_expression(struct parser *parser) {
	struct relational_expression *rel_expr = parse_relational_expression(parser);
	struct equality_expression *eq_expr = equality_expression_init(rel_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_EQ && tok.tok_tag != TOK_NE) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		dynarr_add(eq_expr->oplist, (void *) (long) tok.tok_tag);
		dynarr_add(eq_expr->rel_expr_list, parse_relational_expression(parser));
	}
	return eq_expr;
}

static struct and_expression *parse_and_expression(struct parser *parser) {
	struct equality_expression *eq_expr = parse_equality_expression(parser);
	struct and_expression *and_expr = and_expression_init(eq_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_AMPERSAND) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		dynarr_add(and_expr->eq_expr_list, parse_equality_expression(parser));
	}
	return and_expr;
}

static struct exclusive_or_expression *parse_exclusive_or_expression(struct parser *parser) {
	struct and_expression *and_expr = parse_and_expression(parser);
	struct exclusive_or_expression *xor_expr = exclusive_or_expression_init(and_expr);

	union token tok;
	while (1) {
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag != TOK_XOR) {
			lexer_put_back(parser->lexer, tok);
			break;
		}

		dynarr_add(xor_expr->and_expr_list, parse_and_expression(parser));
	}
	return xor_expr;
}

static struct inclusive_or_expression *parse_inclusive_or_expression(struct parser *parser) {
	struct exclusive_or_expression *ex_expr = parse_exclusive_or_expression(parser);
	panic("parse_inclusive_or_expression ni");
}

static struct logical_and_expression *parse_logical_and_expression(struct parser *parser) {
	struct inclusive_or_expression *or_expr = parse_inclusive_or_expression(parser);
	panic("parse_logical_and_expression ni");
}

static struct logical_or_expression *parse_logical_or_expression(struct parser *parser) {
	struct logical_and_expression *and_expr = parse_logical_and_expression(parser);
	panic("parse_logical_or_expression ni");
}

static struct conditional_expression *parse_conditional_expression(struct parser *parser) {
	struct logical_or_expression *or_expr = parse_logical_or_expression(parser);
	panic("parse_conditional_expression ni");
}

/*
 * assignment_expression
 *   : conditional_expression
 *   | unary_expression assignment_operator assignment_expression
 *   ;
 *
 * It's tricky to find out which alternatives to use.
 *
 * 1) We can always parse conditional_expression first and if the conditional_expression turned
 *    to be a unary_expression and is followed by an assignment_operator, then we select
 *    the second one.
 * 2) We can parse cast_expression which is the first *FORK* node for conditional_expression and 
 *    also a parent of unary_expression
 *
 * XX I'll choose 2) solution since the first one will nest a unary_expression deeply in the 
 * XX expression tree which is not so efficient. We need make expression parsing efficient since
 * XX it's common.
 *
 * I choose 1). 1) has the drawback to nest a unary_expression deeply in the expression tree. However,
 * that's not a problem since anyway this happens a lot: f(a, 1), a and 1 will be nested in
 * assignment expression. The benefit we get is a cleaner function definitions.
 */
static struct assignment_expression *parse_assignment_expression(struct parser *parser) {
	struct conditional_expression *cond_expr = parse_conditional_expression(parser);
	panic("parse_assignment_expression ni");
}

struct expression *parse_expression(struct parser *parser) {
	struct dynarr *darr = dynarr_init();
	union token tok;
	struct assignment_expression *assign_expr = parse_assignment_expression(parser);

	dynarr_add(darr, assign_expr);
	tok = lexer_next_token(parser->lexer);
	while (tok.tok_tag == TOK_COMMA) {
		assign_expr = parse_assignment_expression(parser);	
		dynarr_add(darr, assign_expr);
		tok = lexer_next_token(parser->lexer);
	}
	lexer_put_back(parser->lexer, tok);

	return expression_init(darr);
}
