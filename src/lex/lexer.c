#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <inc/lexer.h>
#include <inc/cbuf.h>
#include <inc/util.h>
#include <inc/keyword.h>

struct lexer *lexer_init(struct file_reader *cstream) {
	struct lexer *lexer = mallocz(sizeof(*lexer));
	lexer->cstream = cstream;
	return lexer;
}

void lexer_destroy(struct lexer *lexer) {
	int i;
	// order is not important here
	for (i = 0; i < lexer->nputback; i++) {
		token_destroy(lexer->putback_stk[i]);
	}
	lexer->nputback = 0;
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
	ptok->const_val.flags = CONST_VAL_TOK_INTEGER;
	ptok->const_val.ival = val;
}

void lexer_put_back(struct lexer *lexer, union token token) {
	assert(lexer->nputback < LOOKAHEAD_NUM);
	lexer->putback_stk[lexer->nputback++] = token;
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
		panic("expect %s, was %s", token_tag_str(tok_tag), token_tag_str(tok.tok_tag));
	}
	return tok;
}

static int handle_bicase(struct lexer *lexer, int follow_ch, int compound_tok, int simple_tok) {
	int ch = file_reader_next_char(lexer->cstream);
	if (ch == follow_ch) {
		return compound_tok;
	} else {
		file_reader_put_back(lexer->cstream, ch);
		return simple_tok;
	}
}

static int handle_tricase(struct lexer *lexer, int folone, int compone, int foltwo, int comptwo, int simple_tok) {
	int ch = file_reader_next_char(lexer->cstream);
	if (ch == folone) {
		return compone;
	} else if (ch == foltwo) {
		return comptwo;
	} else {
		file_reader_put_back(lexer->cstream, ch);
		return simple_tok;
	}
}

static void discard_line(struct lexer *lexer) {
	char ch;
	while ((ch = file_reader_next_char(lexer->cstream)) != '\n') {
	}
}

union token lexer_next_token(struct lexer *lexer) {
	int ch;
	union token tok;
	struct cbuf *cbuf;
	char *s;
	int token_tag;

	if (lexer->nputback > 0) {
		return lexer->putback_stk[--lexer->nputback];
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
	case ',': case ';': case '?': case '~':
	case '[': case ']':
		tok.tok_tag = ch;
		break;
	case '+':
		tok.tok_tag = handle_tricase(lexer, '=', TOK_ADD_ASSIGN, '+', TOK_INC, TOK_ADD);
		break;
	case '-':
		tok.tok_tag = handle_tricase(lexer, '=', TOK_SUB_ASSIGN, '-', TOK_DEC, TOK_SUB);
		break;
	case '%':
		tok.tok_tag = handle_bicase(lexer, '=', TOK_MOD_ASSIGN, TOK_MOD);
		break;
	case '*':
		tok.tok_tag = handle_bicase(lexer, '=', TOK_MUL_ASSIGN, TOK_STAR);
		break;
	case '^':
		tok.tok_tag = handle_bicase(lexer, '=', TOK_XOR_ASSIGN, TOK_XOR);
		break;
	case '=':
		tok.tok_tag = handle_bicase(lexer, '=', TOK_EQ, TOK_ASSIGN);
		break;
	case '!':
		tok.tok_tag = handle_bicase(lexer, '=', TOK_NE, TOK_EXCLAMATION);
	case '/':
		// XXX not support comments yet
		ch = file_reader_next_char(lexer->cstream);
		if (ch == '=') {
			tok.tok_tag = TOK_DIV_ASSIGN;
		} else if (ch == '/') {
			// one line comment
			discard_line(lexer);
			goto repeat;
		} else {
			file_reader_put_back(lexer->cstream, ch);
			tok.tok_tag = TOK_DIV;
		}
		break;
	case '&':
		ch = file_reader_next_char(lexer->cstream);
		if (ch == '&') {
			tok.tok_tag = TOK_LOGIC_AND;
		} else if (ch == '=') {
			tok.tok_tag = TOK_AND_ASSIGN;
		} else {
			file_reader_put_back(lexer->cstream, ch);
			tok.tok_tag = TOK_AMPERSAND;
		}
		break;
	case '|':
		ch = file_reader_next_char(lexer->cstream);
		if (ch == '|') {
			tok.tok_tag = TOK_LOGIC_OR;
		} else if (ch == '=') {
			tok.tok_tag = TOK_OR_ASSIGN;
		} else {
			file_reader_put_back(lexer->cstream, ch);
			tok.tok_tag = TOK_VERT_BAR;
		}
		break;
	case '<':
		ch = file_reader_next_char(lexer->cstream);
		if (ch == '<') {
			ch = file_reader_next_char(lexer->cstream);
			if (ch == '=') {
				tok.tok_tag = TOK_LSHIFT_ASSIGN;
			} else {
				file_reader_put_back(lexer->cstream, ch);
				tok.tok_tag = TOK_LSHIFT;	
			}
		} else if (ch == '=') {
			tok.tok_tag = TOK_LE;
		} else {
			file_reader_put_back(lexer->cstream, ch);
			tok.tok_tag = TOK_LT;
		}
		break;
	case '>':
		ch = file_reader_next_char(lexer->cstream);
		if (ch == '>') {
			ch = file_reader_next_char(lexer->cstream);
			if (ch == '=') {
				tok.tok_tag = TOK_RSHIFT_ASSIGN;
			} else {
				file_reader_put_back(lexer->cstream, ch);
				tok.tok_tag = TOK_RSHIFT;	
			}
		} else if (ch == '=') {
			tok.tok_tag = TOK_GE;
		} else {
			file_reader_put_back(lexer->cstream, ch);
			tok.tok_tag = TOK_GT;
		}
		break;
	case '"':
		parse_string_literal(lexer, &tok);
		break;
	case '.':
		ch = file_reader_next_char(lexer->cstream);
		if (ch == '.') {
			ch = file_reader_next_char(lexer->cstream);
			if (ch == '.') {
				tok.tok_tag = TOK_ELLIPSIS;
			} else {
				panic("invalid '..'");
			}
		} else {
			// TODO handle the .23 case
			panic("invalid '.'");
		}
		break;
	case '#':
		discard_line(lexer); // ignore preprocess right now
		goto repeat;
	default:
		panic("lexer_next_token unexpected character %c", ch);
	}
	// printf("lexer_next_token %d\n", tok.tok_tag);
	return tok;
}

