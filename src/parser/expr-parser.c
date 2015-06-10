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
 * I'll choose 2) solution since the first one will nest a unary_expression deeply in the 
 * expression tree which is not so efficient. We need make expression parsing efficient since
 * it's common.
 */
static struct assignment_expression *parse_assignment_expression(struct parser *parser) {
	struct cast_expression *cast_expression = parse_cast_expression(parser);
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
