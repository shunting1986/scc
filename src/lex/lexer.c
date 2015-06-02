#include <stdlib.h>
#include <stdio.h>
#include <inc/lexer.h>
#include <inc/lexer.h>

struct lexer *lexer_init(struct file_reader *cstream) {
	struct lexer *lexer = malloc(sizeof(*lexer));
	lexer->cstream = cstream;
	return lexer;
}

void lexer_destroy(struct lexer *lexer) {
	free(lexer);
}

void token_destroy(union token token) {
	switch (token.token_tag) {
	case TOK_INT:
		break;
	default:
		panic("token_destroy %d ni", token.token_tag);
		break;
	}
}

void token_dump(union token token) {
	switch (token.token_tag) {
	case TOK_INT:
		printf("tok_int\n");
		break;
	default:
		panic("token_dump %d ni", token.token_tag);
	}
}

union token lexer_next_token(struct lexer *lexer) {
	char ch = file_reader_next_char(lexer->cstream);	
	union token tok;
	switch (ch) {
	case 'a' ... 'z':
	case 'A' ... 'Z':
	case '_': // identifier
		panic("lexer_next_token: identifier parsing ni");

	case EOF:
		tok.token_tag = TOK_EOF;
		break;
	default:
		panic("lexer_next_token unexpected character %c", ch);
	}
	return tok;
}

