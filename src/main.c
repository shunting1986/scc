#include <inc/util.h>
#include <inc/parser.h>
#include <inc/cgasm.h>
#include <inc/cgc.h>

int
main(int argc, char **argv) {
	if (argc != 2) {
		panic("Usage: %s [file to compile]");
	} 
	struct file_reader *fr = file_reader_init(argv[1]);
	if (fr == NULL) {
		panic("file not found %s", argv[1]);
	}
	struct lexer *lexer = lexer_init(fr);
	struct parser *parser = parser_init(lexer);

	struct syntree *tree = parse(parser);

#if 0 
	{
		struct cgc_context *cgctx = cgc_context_init(stderr, 4);
		cgc_tree(cgctx, tree);
		panic("halt");
	}
#endif

	// syntree_dump(tree);
	cgasm_tree(tree);
	syntree_destroy(tree);

	parser_destroy(parser);
	lexer_destroy(lexer);
	file_reader_destroy(fr);

	return 0;
}
