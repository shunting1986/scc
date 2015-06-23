#include <inc/parser.h>
#include <inc/cgasm.h>
#include <gtest/gtest.h>

TEST(parser, parser_to_asm) {
	struct file_reader *fr = file_reader_init("sample-progs/sample.c");
	struct lexer *lexer = lexer_init(fr);
	struct parser *parser = parser_init(lexer);

	struct syntree *tree = parse(parser);

	cgasm_tree(tree);
	syntree_destroy(tree);

	parser_destroy(parser);
	lexer_destroy(lexer);
	file_reader_destroy(fr);
}

TEST(parser, parser_to_c) {
	struct file_reader *fr = file_reader_init("sample-progs/sample.c");
	struct lexer *lexer = lexer_init(fr);
	struct parser *parser = parser_init(lexer);

	struct syntree *tree = parse(parser);

	syntree_dump(tree);
	syntree_destroy(tree);

	parser_destroy(parser);
	lexer_destroy(lexer);
	file_reader_destroy(fr);
}
