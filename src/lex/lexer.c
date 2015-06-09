#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inc/lexer.h>
#include <inc/cbuf.h>
#include <inc/util.h>

struct lexer *lexer_init(struct file_reader *cstream) {
	struct lexer *lexer = malloc(sizeof(*lexer));
	lexer->cstream = cstream;
	lexer->put_back.tok_tag = TOK_UNDEF;
	return lexer;
}

void lexer_destroy(struct lexer *lexer) {
	token_destroy(lexer->put_back);
	free(lexer);
}

/* XXX: escape is not handled yet */
void parse_string_literal(struct lexer *lexer, union token *ptok) {
	struct cbuf *buf = cbuf_init();
	char ch = file_reader_next_char(lexer->cstream);
	while (ch != '"' && ch != EOF) {
		cbuf_add(buf, ch);
		ch = file_reader_next_char(lexer->cstream);
	}
	if (ch == EOF) {
		panic("unterminated string literal");
	}
	ptok->tok_tag = TOK_STRING_LITERAL;
	ptok->str.s = cbuf_transfer(buf);
}

// XXX: only handle decimal integer right now
void parse_number(struct lexer *lexer, union token *ptok) {
	char ch = file_reader_next_char(lexer->cstream);
	int val = 0;
	while (ch >= '0' && ch <= '9') {
		val = val * 10 + (ch - '0');
		ch = file_reader_next_char(lexer->cstream);
	}
	file_reader_put_back(lexer->cstream, ch);
	ptok->tok_tag = TOK_CONSTANT_VALUE;
	ptok->const_val.ival = val;
}

void lexer_put_back(struct lexer *lexer, union token token) {
	assert(lexer->put_back.tok_tag == TOK_UNDEF);
	lexer->put_back = token;
}

// for debugging
void lexer_dump_remaining(struct lexer *lexer) {
	union token tok;
	while (1) {
		tok = lexer_next_token(lexer);
		if (tok.tok_tag == TOK_EOF) {
			break;
		}
		token_dump(tok);
	}
}

union token expect(struct lexer *lexer, int tok_tag) {
	union token tok = lexer_next_token(lexer);
	if (tok.tok_tag != tok_tag) {
		// lexer_dump_remaining(lexer);
		panic("expect %s, was %s", token_tag_str(tok_tag), token_tag_str(tok.tok_tag));
	}
	return tok;
}

union token lexer_next_token(struct lexer *lexer) {
	char ch;
	union token tok;
	struct cbuf *cbuf;
	char *s;
	int token_tag;

	if (lexer->put_back.tok_tag != TOK_UNDEF) {
		union token ret = lexer->put_back;
		memset(&lexer->put_back, 0, sizeof(lexer->put_back));
		lexer->put_back.tok_tag = TOK_UNDEF;
		// printf("lexer_next_token %d\n", tok.tok_tag);
		return ret;
	}

repeat:
	ch = file_reader_next_char(lexer->cstream);	
	switch (ch) {
	case '0' ... '9':
		file_reader_put_back(lexer->cstream, ch);
		parse_number(lexer, &tok);
		break;
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
			tok.tok_tag = TOK_IDENTIFIER;
			tok.id.s = s;
		} else {
			tok.tok_tag = token_tag;
		}
		break;
	case EOF:
		tok.tok_tag = TOK_EOF;
		break;
	case ' ':
	case '\n':
	case '\t':
		goto repeat;
	case '(': case ')': case '{': case '}':
	case ',': case ';': case '&': case '=':
	case '+':
		tok.tok_tag = ch;
		break;
	case '"':
		parse_string_literal(lexer, &tok);
		break;
	default:
		panic("lexer_next_token unexpected character %c", ch);
	}
	// printf("lexer_next_token %d\n", tok.tok_tag);
	return tok;
}

