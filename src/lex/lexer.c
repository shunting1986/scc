#include <stdlib.h>
#include <stdio.h>
#include <inc/lexer.h>
#include <inc/cbuf.h>

struct lexer *lexer_init(struct file_reader *cstream) {
	struct lexer *lexer = malloc(sizeof(*lexer));
	lexer->cstream = cstream;
	return lexer;
}

void lexer_destroy(struct lexer *lexer) {
	free(lexer);
}

void parse_string_literal(union token *ptok) {
	panic("parse_string_literal ni"); // TODO
}

union token lexer_next_token(struct lexer *lexer) {
	char ch;
	union token tok;
	struct cbuf *cbuf;
	char *s;
	int token_tag;

repeat:
	ch = file_reader_next_char(lexer->cstream);	
	switch (ch) {
	case 'a' ... 'z':
	case 'A' ... 'Z':
	case '_': // identifier
		cbuf = cbuf_init();
		do {
			cbuf_add(cbuf, ch);
			ch = file_reader_next_char(lexer->cstream);
		} while (isdigit(ch) || isalpha(ch) || ch == '_');
		file_reader_put_back(lexer->cstream, ch);
		s = cbuf_transfer(cbuf);
		cbuf_destroy(cbuf);

		token_tag = check_keyword_token(s);
		if (token_tag == TOK_UNDEF) {
			// this is an identifier
			tok.token_tag = TOK_IDENTIFIER;
			tok.id_token.s = s;
		} else {
			tok.token_tag = token_tag;
		}
		break;
	case EOF:
		tok.token_tag = TOK_EOF;
		break;
	case ' ':
	case '\n':
	case '\t':
		goto repeat;
	case '(': case ')': case '{': case '}':
	case ',': case ';':
		tok.token_tag = ch;
		break;
	case '"':
		parse_string_literal(&tok);
		break;
	default:
		panic("lexer_next_token unexpected character %c", ch);
	}
	return tok;
}

