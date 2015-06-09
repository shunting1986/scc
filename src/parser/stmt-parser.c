/*
 * Handle the parsing for statement
 */
#include <inc/parser.h>
#include <inc/util.h>

static int initiate_compound_statement(union token tok) {
	panic("initiate_compound_statement ni");
}

static struct compound_statement *parse_compound_statement(struct parser *parser) {
	panic("parse_compound_statement ni"); // TODO: move from parser.c
}

static int initiate_iteration_statement(union token tok) {
	panic("initiate_iteration_statement ni");
}

static struct iteration_statement *parse_iteration_statement(struct parser *parser) {
	panic("parse_iteration_statement ni");
}

static int initiate_jump_statement(union token tok) {
	panic("initiate_jump_statement ni");
}

static struct jump_statement *parse_jump_statement(struct parser *parser) {
	panic("parse_jump_statement ni");
}

static int initiate_selection_statement(union token tok) {
	panic("initiate_selection_statement ni");
}

static struct selection_statement *parse_selection_statement(struct parser *parser) {
	panic("parse_selection_statement ni");
}

static int initiate_labeled_statement(union token tok, union token tok2) {
	panic("initiate_labeled_statement ni");
}

static struct labeled_statement *parse_labeled_statement(struct parser *parser) {
	panic("parse_labeled_statement ni");
}

static struct expression_statement *parse_expression_statement(struct parser *parser) {
	panic("parse_expression_statement ni");
}

/**
 * return the syntreebasenode which can be used as the base type
 */
struct syntreebasenode *parse_statement(struct parser *parser) {
	union token tok = lexer_next_token(parser->lexer);
	if (initiate_compound_statement(tok)) {
		lexer_put_back(parser->lexer, tok);
		return (struct syntreebasenode *)
			parse_compound_statement(parser);
	} else if (initiate_iteration_statement(tok)) {
		lexer_put_back(parser->lexer, tok);
		return (struct syntreebasenode *)
			parse_iteration_statement(parser);
	} else if (initiate_jump_statement(tok)) {
		lexer_put_back(parser->lexer, tok);
		return (struct syntreebasenode *)
			parse_jump_statement(parser);
	} else if (initiate_selection_statement(tok)) {
		lexer_put_back(parser->lexer, tok);
		return (struct syntreebasenode *)
			parse_selection_statement(parser);
	} else {
		// Note: may need 2 lookaheads to decide whether it's a labeled statement 
		// or expression statement
		//
		// a + b
		// a: x = y
		union token tok2 = lexer_next_token(parser->lexer);
		if (initiate_labeled_statement(tok, tok2)) {
			lexer_put_back(parser->lexer, tok2);
			lexer_put_back(parser->lexer, tok);
			return (struct syntreebasenode *)
				parse_labeled_statement(parser);
		} else {
			lexer_put_back(parser->lexer, tok2);
			lexer_put_back(parser->lexer, tok);
			return (struct syntreebasenode *)
				parse_expression_statement(parser);
		}
	}
}


