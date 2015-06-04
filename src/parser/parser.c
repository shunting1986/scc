#include <string.h>
#include <stdlib.h>
#include <inc/parser.h>

struct parser {
	struct lexer *lexer;
};

struct parser *parser_init(struct lexer *lexer) {
	struct parser *parser = malloc(sizeof(*parser));
	parser->lexer = lexer;
	return parser;
}

void parser_destroy(struct parser *parser) {
	free(parser);
}

// assume no EOF found
static struct external_decl_node *parse_external_decl(struct parser *parser) {
	panic("parse_external_decl ni"); // assume no EOF found
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

