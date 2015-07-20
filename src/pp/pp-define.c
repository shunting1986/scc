#include <inc/util.h>
#include <inc/pp.h>

static struct dynarr *store_token_until_newline(struct lexer *lexer) {
	struct dynarr *darr = dynarr_init();
	union token tok;
	while (1) {
		tok = lexer_next_token(lexer);
		if (tok.tok_tag == TOK_NEWLINE) {
			break;
		}

		union token *ptok = malloc(sizeof(*ptok));
		*ptok = tok;
		dynarr_add(darr, ptok);
	}
	return darr;
}

static void pp_define_object_macro(struct lexer *lexer, const char *name) {
	struct dynarr *darr = store_token_until_newline(lexer);
	struct macro *macro = obj_macro_init(darr);
	define_macro(lexer, name, macro);
}

void pp_define(struct lexer *lexer) {
	union token idtok = expect(lexer, TOK_IDENTIFIER);

	// we do not use lexer_next_token to handle space correctly
	char ch = file_reader_next_char(lexer->cstream);
	if (ch != EOF) { // peek
		file_reader_put_back(lexer->cstream, ch);
	}

	if (ch == '(') {
		panic("function macro not supported yet");
	} else {
		pp_define_object_macro(lexer, idtok.id.s);
		token_destroy(idtok);
	}
}
