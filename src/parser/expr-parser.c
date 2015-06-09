/*
 * Handle the parsing for expression
 */
#include <inc/parser.h>
#include <inc/lexer.h>
#include <inc/util.h>
#include <inc/dynarr.h>

struct assignment_expression *parse_assignment_expression(struct parser *parser) {
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
