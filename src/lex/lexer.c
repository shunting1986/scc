#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <inc/lexer.h>
#include <inc/cbuf.h>
#include <inc/util.h>
#include <inc/keyword.h>
#include <inc/pp.h>
#include <inc/htab.h>

#undef DEBUG
#define DEBUG 1

static char lexer_next_char(struct lexer *lexer);

struct lexer *lexer_init(struct file_reader *cstream) {
	struct lexer *lexer = mallocz(sizeof(*lexer));
	lexer->cstream = cstream;
	lexer->typedef_tab = typedef_tab_init(NULL);

	// setup macro tab
	lexer->macro_tab = htab_init();
	lexer->macro_tab->val_free_fn = macro_destroy;

	lexer->expanded_macro = dynarr_init();
	return lexer;
}

void normalize_expanded_token_list(struct lexer *lexer) {
	if (lexer->expanded_macro_pos == dynarr_size(lexer->expanded_macro)) {
		DYNARR_FOREACH_PLAIN_BEGIN(lexer->expanded_macro, union token *, each);
			assert(each == NULL);
		DYNARR_FOREACH_END();
		dynarr_clear(lexer->expanded_macro);
		lexer->expanded_macro_pos = 0;
	}
}

void lexer_destroy(struct lexer *lexer) {
	int i;
	// order is not important here
	for (i = 0; i < lexer->nputback; i++) {
		token_destroy(lexer->putback_stk[i]);
	}
	lexer->nputback = 0;
	lexer_pop_typedef_tab(lexer);

	// free macro tab
	htab_destroy(lexer->macro_tab);

	assert(lexer->expanded_macro_pos == dynarr_size(lexer->expanded_macro));
	normalize_expanded_token_list(lexer);
	dynarr_destroy(lexer->expanded_macro);

	free(lexer);
}

/* XXX: escape is not handled yet */
/* XXX: caller should take care of freeing the memory */
char *parse_string_literal(struct lexer *lexer, int term_tag) {
	struct cbuf *buf = cbuf_init();
	char ch;

	// handle the \" case so that we can get the whole quoted string
	bool follow_backslash = false;

	while (true) {
		ch = file_reader_next_char(lexer->cstream);
		if (ch == EOF || (!follow_backslash && ch == term_tag)) {
			break;
		}

		cbuf_add(buf, ch);

		if (ch == '\\') {
			follow_backslash = !follow_backslash; // two consecutive backslash will not
				// escape following character
		} else {
			follow_backslash = false;
		}
	}
	if (ch == EOF) {
		panic("unterminated string literal");
	}
	char *ret = cbuf_transfer(buf);
	cbuf_destroy(buf);
	return ret;
}

// XXX right now we treat single quote as int constant	
void parse_single_quote(struct lexer *lexer, union token *ptok) {
	char buf[11]; // 10 should be enough
	int size = 0;
	char ch;
	int val = 0;
	int i;

	while ((ch = file_reader_next_char(lexer->cstream)) != EOF && ch != '\'') {
		if (size == 10) {
			panic("Invalid single quotation: enclosing too many characters");
		}
		buf[size++] = ch;
	}

	if (ch == EOF) {
		panic("Unterminated \"'\"");
	}

	buf[size] = '\0';

	if (buf[0] == '\\') {
		if (size == 1) {
			panic("invalid usage of backslash");
		}
		if (buf[1] >= '0' && buf[1] <= '7') {
			val = buf[1] - '0';
			for (i = 2; i < size; i++) {
				if (buf[i] >= '0' && buf[i] <= '7') {
					val = val * 8 + (buf[i] - '0');
				} else {
					break;
				}
			}
			if (i < size) {
				panic("invalid usage of backslash");
			}
		} else if (buf[1] == 'x' || buf[1] == 'X') {
			if (size == 2) {
				panic("invalid single quotation: \\x should follow hexadecimal number");
			}
			for (i = 2; i < size; i++) {
				if (is_hex_char(buf[i])) {
					val = val * 16 + get_hex_value_from_char(buf[i]);
				}
			}
			if (i < size) {
				panic("invalid usage of backslash");
			}
		} else if (size == 2) {
			switch (buf[1]) {
			case '\\': val = '\\'; break;
			default:
				goto invalid;
			}
		} else {
		invalid:
			file_reader_dump_remaining(lexer->cstream);
			panic("unsupported/invalid single quotation %s", buf);
		}
	} else if (size == 1) {
		val = (unsigned char) buf[0];
	} else {
		panic("unsupported/invalid single quotation");
	}

	ptok->tok_tag = TOK_CONSTANT_VALUE;
	ptok->const_val.flags = CONST_VAL_TOK_INTEGER;
	ptok->const_val.ival = val;
}

// XXX: only handle decimal & hexadecima integer right now
void parse_number(struct lexer *lexer, union token *ptok) {
	char ch = file_reader_next_char(lexer->cstream);
	int base = 10;
	assert(ch >= '0' && ch <= '9');
	char peek = file_reader_next_char(lexer->cstream);
	if (peek == 'x' || peek == 'X') {
		ch = file_reader_next_char(lexer->cstream);
		if (!is_hex_char(ch)) {
			panic("Invalid hexadecimal number");
		}
		base = 16;
	} else {
		file_reader_put_back(lexer->cstream, peek);
	}

	unsigned long long val = 0;
	while ((ch >= '0' && ch <= '9') || (base == 16 && is_hex_char(ch))) {
		val = val * base + get_hex_value_from_char(ch);
		ch = file_reader_next_char(lexer->cstream);
	}

	// stat the following u and l
	int num_u = 0;
	int num_l = 0;
	while (ch == 'u' || ch == 'U' || ch == 'l' || ch == 'L') {
		num_u += ch == 'u' || ch == 'U';
		num_l += ch == 'l' || ch == 'L';
		ch = file_reader_next_char(lexer->cstream);
	}
	file_reader_put_back(lexer->cstream, ch);

	// XXX ignore num_u right now
	if (num_l >= 3) {
		panic("too many following 'l'");
	}

	ptok->tok_tag = TOK_CONSTANT_VALUE;

	if (num_l < 2) {
		ptok->const_val.flags = CONST_VAL_TOK_INTEGER;
		ptok->const_val.ival = 0;

		// ignore the overflow error if we are in skip mode
		if (!lexer->in_skip_mode) {
			if (val & 0xFFFFFFFF00000000ll) {
				file_reader_dump_remaining(lexer->cstream);
				panic("Constant exceed 32 bit range");
			}
			ptok->const_val.ival = (int) val; 
		}
	} else {
		ptok->const_val.flags = CONST_VAL_TOK_LONG_LONG;
		ptok->const_val.llval = val; 
	}
}

void lexer_put_back(struct lexer *lexer, union token token) {
	assert(lexer->nputback < LOOKAHEAD_NUM);
	lexer->putback_stk[lexer->nputback++] = token;
}

// for debugging
void lexer_dump_remaining(struct lexer *lexer) {
	union token tok;
	int tot = 0;
	// NOTE: the path may be suitable only for the beginning several tokens
	fprintf(stderr, "[%s] dump remaining tokens\n", lexer->cstream->path);
	while (1) {
		tok = lexer_next_token(lexer);
		if (tok.tok_tag == TOK_EOF) {
			break;
		}
		token_dump(tok);
		// if (++tot >= 20) { // dump the first 20
		if (++tot >= 10) { // dump the first 20
			break;
		}
	}
}

union token expect(struct lexer *lexer, int tok_tag) {
	union token tok = lexer_next_token(lexer);
	if (tok.tok_tag != tok_tag) {
		file_reader_dump_remaining(lexer->cstream); // TODO
		token_dump(tok);
		panic("expect %s, was %s", token_tag_str(tok_tag), token_tag_str(tok.tok_tag));
		assert(false); // TODO
	}
	return tok;
}

void assume(union token tok, int tok_tag) {
	if (tok.tok_tag != tok_tag) {
		panic("expect %s, was %s", token_tag_str(tok_tag), token_tag_str(tok.tok_tag));
	}
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

static int handle_quacase(struct lexer *lexer, int folone, int tokone, int foltwo, int toktwo, int folthree, int tokthree, int simple_tok) {
	int ch = file_reader_next_char(lexer->cstream);
	if (ch == folone) {
		return tokone;
	} else if (ch == foltwo) {
		return toktwo;
	} else if (ch == folthree) {
		return tokthree;
	} else {
		file_reader_put_back(lexer->cstream, ch);
		return simple_tok;
	}
}

void lexer_discard_line(struct lexer *lexer) {
	char ch;
	while ((ch = file_reader_next_char(lexer->cstream)) != '\n') {
	}

	// NOTE: we purposely put back the '\n', so that the pp module can identify the '#'
	// at the begining of a line
	file_reader_put_back(lexer->cstream, '\n');
}

static void parse_multi_line_comment(struct lexer *lexer) {
	// NOTE: use file_reader_next_char instead of lexer_next_char since
	// we do not allow the comments spread thru multi files
	char ch = '\0';
	char nxtch = file_reader_next_char(lexer->cstream);
	if (ch == EOF) {
		panic("unterminated comment");
	}

	while (!(ch == '*' && nxtch == '/')) {
		ch = nxtch;
		nxtch = file_reader_next_char(lexer->cstream);
		if (nxtch == EOF) {
			panic("unterminated comment");
		}
	}
}

static char lexer_next_char(struct lexer *lexer) {
	char ch;
repeat:
	ch = file_reader_next_char(lexer->cstream);	
	if (ch == EOF) {
		if (lexer->cstream->if_nest_level > 0) {
			panic("#if not paired");
		}
		struct file_reader *cur = lexer->cstream;
#if DEBUG
		fprintf(stderr, "leave file %s\n", lexer->cstream->path); 
#endif
		if (cur->prev != NULL) {
			lexer->cstream = cur->prev;
			file_reader_destroy(cur);
			goto repeat;
		}
	}
	return ch;
}

static bool has_more_expanded_token(struct lexer *lexer) {
	return lexer->expanded_macro_pos < dynarr_size(lexer->expanded_macro);
}

static union token obtain_next_expanded_token(struct lexer *lexer) {
	assert(has_more_expanded_token(lexer));
	union token *ptr = dynarr_get(lexer->expanded_macro, lexer->expanded_macro_pos);
	dynarr_set(lexer->expanded_macro, lexer->expanded_macro_pos++, NULL);
	union token ret = *ptr;
	free(ptr);
	return ret;
}

/*
 * Only return the tag to simplify memory manangement for id token etc.
 */
static int lexer_peek_token(struct lexer *lexer) {
	union token tok = lexer_next_token(lexer);
	int tag = tok.tok_tag;
	lexer_put_back(lexer, tok);
	return tag;
}

union token lexer_next_token(struct lexer *lexer) {
	int ch;
	union token tok;
	struct cbuf *cbuf;
	char *s;
	int token_tag;

repeat:
	tok.tok_tag = TOK_UNDEF;
	if (lexer->nputback > 0) {
		tok = lexer->putback_stk[--lexer->nputback];
	}

	// Take care one special case. We define a macro, this macro contains an id. Later 
	// the id is defined as a typename. We we expand the macro, the id should be return
	// as type name rather than id!
	if (tok.tok_tag == TOK_UNDEF && has_more_expanded_token(lexer)) {
		tok = obtain_next_expanded_token(lexer);
	}

	if (tok.tok_tag != TOK_UNDEF) {
		if (tok.tok_tag != TOK_IDENTIFIER) {
			goto out;
		} else {
			goto check_id_token; // handle typedef or recursive expanding macro
		}
	}

	ch = lexer_next_char(lexer);
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

		if (lexer->in_pp_context) {
			token_tag = check_pp_keyword(s);	
			if (token_tag != TOK_UNDEF) {
				tok.tok_tag = token_tag;
				free(s);
				break;
			}
		}

		token_tag = check_keyword_token(s);
		int peek_tok;
		if (token_tag != TOK_UNDEF) {
			tok.tok_tag = token_tag;
			free(s);
			break;
		} 
		tok.tok_tag = TOK_IDENTIFIER;
		tok.id.s = s;

check_id_token:
		// below check_id_token, use tok.id.s instead of s since s is not always valid
		// from all the paths
		if (lexer_is_typedef(lexer, tok.id.s) && (peek_tok = lexer_peek_token(lexer)) != TOK_COMMA && peek_tok != TOK_SEMICOLON) {
			tok.tok_tag = TOK_TYPE_NAME;
		} else if (try_expand_macro(lexer, tok.id.s)) {
			// this is a macro
			free(tok.id.s);
			goto repeat;
		} 

		break;
	case EOF:
		tok.tok_tag = TOK_EOF;
		break;
	case '\n':
		if (lexer->want_newline) {
			tok.tok_tag = TOK_NEWLINE;
			break;
		} else {
			goto repeat;
		}
	case ' ':
	case '\t':
	case '\r':
	case '\f': // some header file may contain this character
		goto repeat;
	case '(': case ')': case '{': case '}':
	case ',': case ';': case '?': case '~':
	case '[': case ']': case ':': 
		tok.tok_tag = ch;
		break;
	case '#':
		tok.tok_tag = handle_bicase(lexer, '#', TOK_BISHARP, TOK_SHARP);
		break;
	case '\\':
		// handle stray
		ch = file_reader_next_char(lexer->cstream);
		if (ch == '\n') {
			goto repeat;
		} else {
			file_reader_put_back(lexer->cstream, ch);

			file_reader_dump_remaining(lexer->cstream);
			panic("Invalid '\\'");
		}
		break;

	case '+':
		tok.tok_tag = handle_tricase(lexer, '=', TOK_ADD_ASSIGN, '+', TOK_INC, TOK_ADD);
		break;
	case '-':
		tok.tok_tag = handle_quacase(lexer, '=', TOK_SUB_ASSIGN, '-', TOK_DEC, '>', TOK_PTR_OP, TOK_SUB);
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
		break;
	case '/':
		ch = file_reader_next_char(lexer->cstream);
		if (ch == '=') {
			tok.tok_tag = TOK_DIV_ASSIGN;
		} else if (ch == '/') {
			// one line comment
			lexer_discard_line(lexer);
			goto repeat;
		} else if (ch == '*') {
			parse_multi_line_comment(lexer);
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
	case '\'':
		parse_single_quote(lexer, &tok);
		break;
	case '"':
		if (lexer->want_quotation) { // in pp context, return " directly so the processing is similar to < include
			tok.tok_tag = TOK_QUOTATION;
		} else {
			char *s = parse_string_literal(lexer, TOK_QUOTATION);
			tok.tok_tag = TOK_STRING_LITERAL;
			tok.str.s = s;
		}
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
			file_reader_put_back(lexer->cstream, ch);
			tok.tok_tag = TOK_DOT; // struct_obj.field case
		}
		break;
	default:
#if DEBUG
		file_reader_dump_remaining(lexer->cstream); 
#endif
		panic("lexer_next_token unexpected character '%c'", ch);
	}

out:
	// We need this special handling for sharp since we may put back sharp token
	// to the lexer in pp
	if (tok.tok_tag == TOK_SHARP && !lexer->want_sharp) {
		pp_entry(lexer);
		goto repeat; 
	}

	return tok;
}

