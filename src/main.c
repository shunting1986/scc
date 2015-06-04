#include <inc/util.h>
#include <inc/parser.h>

int
main(int argc, char **argv) {
	/*
	if (argc != 2) {
		panic("Usage: %s [file to compile]");
	} 
	 */
	struct file_reader *fr = file_reader_init("sample-progs/sample.c");
	struct lexer *lexer = lexer_init(fr);
	struct parser *parser = parser_init(lexer);

	struct syntree *tree = parse(parser);

	syntree_dump(tree);

	parser_destroy(parser);
	lexer_destroy(lexer);
	file_reader_destroy(fr);

	return 0;
}
