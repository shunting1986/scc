#include <inc/lexer.h>
#include <gtest/gtest.h>

TEST(lexer, lexer) {
	struct file_reader *fr = file_reader_init("sample-progs/sample.c");
	struct lexer *lexer = lexer_init(fr);

	union token tok;
	while ((tok = lexer_next_token(lexer)).tok_tag != TOK_EOF) {
		token_dump(tok);
		token_destroy(tok);
	}

	lexer_destroy(lexer);
	file_reader_destroy(fr);
}
