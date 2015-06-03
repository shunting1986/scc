#include <inc/parser.h>
#include <gtest/gtest.h>

TEST(parser, parser) {
	struct file_reader *fr = file_reader_init("sample-progs/sample.c");
	struct lexer *lexer = lexer_init(fr);
	struct parser *parser = parser_init(lexer);

	struct syntree *tree = parse(parser);

	syntree_dump(tree);

	parser_destroy(parser);
	lexer_destroy(lexer);
	file_reader_destroy(fr);
}
