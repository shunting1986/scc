#include <inc/util.h>
#include <inc/pp.h>

#undef DEBUG
#define DEBUG 1

static struct dynarr *store_token_until_newline(struct lexer *lexer) {
	struct dynarr *darr = dynarr_init();
	union token tok;
	while (1) {
		tok = lexer_next_token(lexer);
		if (tok.tok_tag == TOK_NEWLINE) {
			break;
		}

		union token *ptok = mallocz(sizeof(*ptok));
		*ptok = tok;
		dynarr_add(darr, ptok);
	}
	return darr;
}

static void pp_define_object_macro(struct lexer *lexer, const char *name) {
	struct dynarr *darr = store_token_until_newline(lexer);
	struct macro *macro = obj_macro_init(darr);
	define_macro(lexer, name, macro);

#if DEBUG
	// fprintf(stderr, "%s define the macro %s\n", lexer->cstream->path, name);
	macro_dump(name, macro);
#endif
}

/*
 * not support '...' yet
 */
static void pp_define_func_macro(struct lexer *lexer, const char *name) {
	expect(lexer, TOK_LPAREN);
	struct dynarr *paramlist = dynarr_init();

	union token tok = lexer_next_token(lexer); 
	if (tok.tok_tag != TOK_RPAREN) {
		while (true) {
			assume(tok, TOK_IDENTIFIER);
			dynarr_add(paramlist, tok.id.s);
			tok = lexer_next_token(lexer);
			if (tok.tok_tag == TOK_RPAREN) {
				break;
			} else {
				assume(tok, TOK_COMMA);
			}
			tok = lexer_next_token(lexer);
		}
	}
	struct dynarr *darr = store_token_until_newline(lexer);
	struct macro *macro = func_macro_init(paramlist, darr);
	define_macro(lexer, name, macro);

#if DEBUG
	macro_dump(name, macro);
#endif
}

void pp_define(struct lexer *lexer) {
	int old_want_sharp = lexer_push_config(lexer, want_sharp, 1);
	// TODO: note this does not check for the skip mode yet
	union token idtok = expect(lexer, TOK_IDENTIFIER);

	// we do not use lexer_next_token to handle space correctly
	char ch = file_reader_next_char(lexer->cstream);
	if (ch != EOF) { // peek
		file_reader_put_back(lexer->cstream, ch);
	}

	if (ch == '(') {
		pp_define_func_macro(lexer, idtok.id.s);
	} else {
		pp_define_object_macro(lexer, idtok.id.s);
	}
	token_destroy(idtok);
	lexer_pop_config(lexer, want_sharp, old_want_sharp);
}

void pp_undef(struct lexer *lexer) {
	union token idtok = expect(lexer, TOK_IDENTIFIER);
	undef_macro(lexer, idtok.id.s);
	token_destroy(idtok);
}


