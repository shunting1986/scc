#include <stdio.h>
#include <inc/util.h>
#include <inc/pp.h>
#include <inc/lexer.h>
#include <inc/file_reader.h>

#define DEBUG 1

static void pp_ifndef(struct lexer *lexer) {
	int old_want_newline = push_want_newline(lexer, 1);
	
	union token tok = expect(lexer, TOK_IDENTIFIER);
	union token nxtok = expect(lexer, TOK_NEWLINE);
	(void) nxtok;

	int flags = pp_in_skip_mode(lexer) ? IF_FLAG_ALWAYS_SKIP : 0;
	if (macro_defined(lexer, tok.id.s)) {
		pp_push_if_item(lexer, IF_FALSE, flags);
	} else {
		pp_push_if_item(lexer, IF_TRUE, flags);
	}

	pop_want_newline(lexer, old_want_newline);
}

static void pp_if(struct lexer *lexer) {
	int old_want_newline = push_want_newline(lexer, 1);
	int result = pp_expr(lexer);
	int flags = pp_in_skip_mode(lexer) ? IF_FLAG_ALWAYS_SKIP : 0;
	if (result) {
		pp_push_if_item(lexer, IF_TRUE, flags);
	} else {
		pp_push_if_item(lexer, IF_FALSE, flags);
	}

	pop_want_newline(lexer, old_want_newline);
}

void pp_entry(struct lexer *lexer) {
	int old_in_pp_context = lexer->in_pp_context;
	lexer->in_pp_context = 1;

	union token tok = lexer_next_token(lexer);

	switch (tok.tok_tag) {
	case PP_TOK_INCLUDE:
		pp_include(lexer);
		break;
	case PP_TOK_IFNDEF:
		pp_ifndef(lexer);
		break;
	case PP_TOK_IF:
		pp_if(lexer);
		break;
	default:
#if DEBUG
		// lexer_dump_remaining(lexer);
#endif
		panic("ni %s", token_tag_str(tok.tok_tag));
	}

	lexer->in_pp_context = old_in_pp_context;
}
