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

static struct translation_unit_node *parse_translation_unit(struct parser *parser) {
	panic("parse_translation_unit"); // assume no EOF found
}

struct syntree *parse(struct parser *parser) {
	union token tok;
	while ((tok = lexer_next_token(parser->lexer)).tok_tag != TOK_EOF) {
		// lexer put back, should not destroy tok
		lexer_put_back(tok);
		struct translation_unit_node *unit_node = parse_translation_unit(parser);
	}
	panic("parse ni"); // TODO
}

void parser_destroy(struct parser *parser) {
	panic("parser_destroy ni"); // TODO
}
