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

// TODO: I plan to have a function to skip all the '#if false' code so that we do not
// need to check for 'skip mode' everywhere
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

	fprintf(stderr, "\033[31mHave a central place to handle skip mode\033[0m\n"); // TODO

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
	case PP_TOK_DEFINE:
		pp_define(lexer);
		break;
	case PP_TOK_UNDEF:
		pp_undef(lexer);
		break;
	default:
#if DEBUG
		// lexer_dump_remaining(lexer);
#endif
		token_dump(tok);
		panic("ni %s", token_tag_str(tok.tok_tag));
	}

	lexer->in_pp_context = old_in_pp_context;
}
