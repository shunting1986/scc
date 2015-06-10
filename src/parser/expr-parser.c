/*
 * Handle the parsing for expression
 */
#include <inc/parser.h>
#include <inc/lexer.h>
#include <inc/util.h>
#include <inc/dynarr.h>

static struct unary_expression *parse_unary_expression(struct parser *parser) {
	panic("parse_unary_expression ni");
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
	panic("parse_multiplicative_expression ni");
}

static struct additive_expression *parse_additive_expression(struct parser *parser) {
	struct multiplicative_expression *mul_expr = parse_multiplicative_expression(parser);
	panic("parse_additive_expression ni");
}

static struct shift_expression *parse_shift_expression(struct parser *parser) {
	struct additive_expression *add_expr = parse_additive_expression(parser);
	panic("parse_shift_expression ni");
}

static struct relational_expression *parse_relational_expression(struct parser *parser) {
	struct shift_expression *shift_expr = parse_shift_expression(parser);
	panic("parse_relational_expression ni");
}

static struct equality_expression *parse_equality_expression(struct parser *parser) {
	struct relational_expression *rel_expr = parse_relational_expression(parser);
	panic("parse_equality_expression ni");
}

static struct and_expression *parse_and_expression(struct parser *parser) {
	struct equality_expression *eq_expr = parse_equality_expression(parser);
	panic("parse_and_expression ni");
}

static struct exclusive_or_expression *parse_exclusive_or_expression(struct parser *parser) {
	struct and_expression *and_expr = parse_and_expression(parser);
	panic("parse_exclusive_or_expression ni");
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
