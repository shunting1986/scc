/*
 * Handle the parsing for statement
 */
#include <inc/parser.h>
#include <inc/util.h>
#include <inc/dynarr.h>

static int initiate_compound_statement(union token tok) {
	return tok.tok_tag == TOK_LBRACE;
}

struct compound_statement *parse_compound_statement(struct parser *parser) {
	expect(parser->lexer, TOK_LBRACE);

	// look one token ahead to determing if this is a declaration or statement or empty block
	union token tok = lexer_next_token(parser->lexer);
	struct dynarr *declList = dynarr_init();
	struct dynarr *stmtList = dynarr_init();
	while (tok.tok_tag != TOK_RBRACE) {
		if (initiate_declaration(tok)) {
			if (dynarr_size(stmtList) > 0) {
				panic("encounter declaration after statement");
			}
			lexer_put_back(parser->lexer, tok);
			dynarr_add(declList, parse_declaration(parser));
		} else { // initiate a statement
			lexer_put_back(parser->lexer, tok);
			dynarr_add(stmtList, parse_statement(parser));
		}
		tok = lexer_next_token(parser->lexer);
	}
	return compound_statement_init(declList, stmtList);
}

static int initiate_iteration_statement(union token tok) {
	return tok.tok_tag == TOK_WHILE || tok.tok_tag == TOK_FOR || tok.tok_tag == TOK_DO;
}

static struct iteration_statement *parse_iteration_statement(struct parser *parser) {
	panic("parse_iteration_statement ni");
}

static int initiate_jump_statement(union token tok) {
	return tok.tok_tag == TOK_GOTO || tok.tok_tag == TOK_CONTINUE
			|| tok.tok_tag == TOK_BREAK || tok.tok_tag == TOK_RETURN;
}

static struct jump_statement *parse_jump_statement(struct parser *parser) {
	panic("parse_jump_statement ni");
}

static int initiate_selection_statement(union token tok) {
	return tok.tok_tag == TOK_IF || tok.tok_tag == TOK_SWITCH;
}

static struct selection_statement *parse_selection_statement(struct parser *parser) {
	panic("parse_selection_statement ni");
}

static int initiate_labeled_statement(union token tok, union token tok2) {
	return tok.tok_tag == TOK_CASE || tok.tok_tag == TOK_DEFAULT ||
		(tok.tok_tag == TOK_IDENTIFIER && tok2.tok_tag == TOK_COLON);
}

static struct labeled_statement *parse_labeled_statement(struct parser *parser) {
	panic("parse_labeled_statement ni");
}

static struct expression_statement *parse_expression_statement(struct parser *parser) {
	union token tok = lexer_next_token(parser->lexer);
	struct expression *expr;
	if (tok.tok_tag == TOK_SEMICOLON) {
		expr = NULL;
	} else {
		lexer_put_back(parser->lexer, tok);
		expr = parse_expression(parser);
		expect(parser->lexer, TOK_SEMICOLON);
	}
	return expression_statement_init(expr);
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


