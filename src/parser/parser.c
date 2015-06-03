#include <inc/parser.h>

struct parser {
	struct lexer *lexer;
};

struct parser *parser_init(struct lexer *lexer) {
	panic("parser_init ni"); // TODO
}

struct syntree *parse(struct parser *parser) {
	panic("parse ni"); // TODO
}

void parser_destroy(struct parser *parser) {
	panic("parser_destroy ni"); // TODO
}
