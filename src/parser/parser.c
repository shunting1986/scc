#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <inc/parser.h>
#include <inc/dynarr.h>

struct parser {
	struct lexer *lexer;
};

static struct external_decl_node *parse_external_decl(struct parser *parser);

struct parser *parser_init(struct lexer *lexer) {
	struct parser *parser = malloc(sizeof(*parser));
	parser->lexer = lexer;
	return parser;
}

void parser_destroy(struct parser *parser) {
	free(parser);
}

struct syntree *parse(struct parser *parser) {
	union token tok;
	while ((tok = lexer_next_token(parser->lexer)).tok_tag != TOK_EOF) {
		// lexer put back, should not destroy tok
		lexer_put_back(parser->lexer, tok);
		struct external_decl_node *external_decl = parse_external_decl(parser);
	}
	panic("parse ni"); // TODO
}

static struct declaration_specifiers *parse_decl_specifiers(struct parser *parser) {
	union token tok;
	void *nd;
	struct dynarr *darr = dynarr_init();

	while (1) { // TODO handle more cases
		tok = lexer_next_token(parser->lexer);
		if (tok.tok_tag == TOK_INT) {
			nd = type_specifier_init(tok.tok_tag);
			token_destroy(tok);
		} else {
			lexer_put_back(parser->lexer, tok);
			break;
		}
		dynarr_add(darr, nd);
	}

	// the list should not be empty
	assert(dynarr_size(darr) > 0);
	return declaration_specifiers_init(darr);
}

// assume no EOF found; 
// XXX: only handle function definition right now
static struct external_decl_node *parse_external_decl(struct parser *parser) {
	struct declaration_specifiers *decl_specifiers = parse_decl_specifiers(parser);
	panic("parse_external_decl ni"); // assume no EOF found
}

