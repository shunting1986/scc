#include <inc/util.h>
#include <inc/pp.h>

#undef DEBUG
#define DEBUG 0

#define DUMP_MACRO 0

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

	// simple check for: #define x x case.
	// a real example is in: /usr/include/bits/confname.h
	//
	// the ultimate way to sovle the (indirectly) referring itself obj/func macro is 
	// constructing the macro expanding tree
	if (dynarr_size(darr) == 1) {
		union token *tok = dynarr_get(darr, 0);
		if (tok->tok_tag == TOK_IDENTIFIER && strcmp(tok->id.s, name) == 0) {
			token_destroy(*tok);
			free(tok);
			dynarr_destroy(darr);
			red("ignore identity obj macro %s", name);
			return;
		}
	}
	struct macro *macro = obj_macro_init(darr);
	define_macro(lexer, name, macro);

#if DUMP_MACRO
	// fprintf(stderr, "%s define the macro %s\n", lexer->cstream->path, name);
	macro_dump(lexer, name, macro);
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

#if DUMP_MACRO
	macro_dump(lexer, name, macro);
#endif
}

void pp_define(struct lexer *lexer) {
	int old_want_sharp = lexer_push_config(lexer, want_sharp, 1);

	// must enable want_space befoer get id token since parsing id will peek
	int old_want_space = lexer_push_config(lexer, want_space, 1);
	union token idtok;

	while (true) {
		idtok = lexer_next_token(lexer);
		if (idtok.tok_tag != TOK_SPACE && idtok.tok_tag != TOK_TAB) {
			break;	
		}
	}
	assume(idtok, TOK_IDENTIFIER);
	union token peektok = lexer_next_token(lexer);
	lexer_pop_config(lexer, want_space, old_want_space);

	if (peektok.tok_tag == TOK_LPAREN) {
		lexer_put_back(lexer, peektok);
		pp_define_func_macro(lexer, idtok.id.s);
	} else {
		if (peektok.tok_tag != TOK_SPACE && peektok.tok_tag != TOK_TAB) {
			lexer_put_back(lexer, peektok);
		}
		pp_define_object_macro(lexer, idtok.id.s);
	}
	token_destroy(idtok);
	lexer_pop_config(lexer, want_sharp, old_want_sharp);
}

void pp_cmdline_define(struct lexer *lexer, const char *id) {
	struct dynarr *darr = dynarr_init();
	union token int_const_tok = wrap_int_const_to_token(1);
	dynarr_add(darr, token_shallow_dup(&int_const_tok));
	struct macro *macro = obj_macro_init(darr);
	define_macro(lexer, id, macro);
}

void pp_undef(struct lexer *lexer) {
	union token idtok = expect(lexer, TOK_IDENTIFIER);
	undef_macro(lexer, idtok.id.s);
	token_destroy(idtok);
}


