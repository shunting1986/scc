#include <inc/util.h>
#include <inc/parser.h>
#include <inc/cgasm.h>
#include <inc/cgc.h>
#include <inc/pp.h>

void parse_options(struct lexer *lexer, int argc, char **argv) {
	int i;
	for (i = 1; i < argc - 1; i++) {
		char *s = argv[i];

		if (s[0] != '-' || s[1] != 'D') {
			panic("only support -D right now");
		}
		if (!valid_id_str(s + 2)) {
			panic("require id after -D");
		}
		pp_cmdline_define(lexer, s + 2);
	}
}

int
main(int argc, char **argv) {
	if (argc < 2) {
		panic("Usage: %s [file to compile]", argv[0]);
	} 

	struct file_reader *fr = file_reader_init(argv[argc - 1]);
	if (fr == NULL) {
		panic("file not found %s", argv[argc - 1]);
	}

	struct lexer *lexer = lexer_init(fr);
	parse_options(lexer, argc, argv);
	struct parser *parser = parser_init(lexer);

	struct syntree *tree = parse(parser);

#if 1
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
